/*
 *  input handling
 */

#include <ctype.h>         /* isdigit */
#include <limits.h>        /* ULONG_MAX */
#include <stddef.h>        /* NULL */
#include <stdio.h>         /* FILE, BUFSIZ, fgets, ferror, feof, fread, getc, ungetc */
#include <string.h>        /* strlen, strncmp */
#include <cbl/assert.h>    /* assert */
#include <cbl/memory.h>    /* MEM_NEW, MEM_ALLOC, MEM_RESIZE, MEM_FREE */
#include <cdsl/hash.h>     /* hash_string, hash_new */

#include "common.h"
#include "err.h"
#include "main.h"
#ifdef SEA_CANARY
#include "../cpp/inc.h"
#include "../cpp/lex.h"
#else    /* !SEA_CANARY */
#include "lex.h"
#endif    /* SEA_CANARY */
#include "in.h"

#if BUFSIZ > TL_LINE
#define BUFSIZE BUFSIZ         /* (!usedynamic) input buffer size */
#else    /* BUFSIZ <= TL_LINE */
#define BUFSIZE (TL_LINE*2)    /* (!usedynamic) input buffer size */
#endif    /* BUFSIZ > TL_LINE */
#define BUFLEN  BUFUNIT        /* (usedynamic) unit size of input buffer */

/* true if in_nextline can be set to nextlined() */
#ifdef SEA_CANARY
#define usedynamic 1
#else    /* !SEA_CANARY */
#define usedynamic (main_opt()->diagstyle == 1)
#endif    /* SEA_CANARY */


in_pos_t in_cpos;                /* current input file locus */
const unsigned char *in_line;    /* beginning of current line */
const unsigned char *in_cp;      /* current character */
const unsigned char *in_limit;   /* end of current input buffer */
unsigned long in_outlen;         /* length of deleted part (0 when usedynamic) */
void (*in_nextline)(void);       /* function to read next input line */
#ifndef SEA_CANARY
in_inc_t **in_incp;              /* #include list */
#endif    /* !SEA_CANARY */


static FILE *fptr;                                     /* file pointer for input */
static int bsize;                                      /* buffer status (0 when usedynamic) */
#ifdef HAVE_ICONV
static unsigned char *buf;                             /* input buffer */
#else    /* !HAVE_ICONV */
static unsigned char buf[1+IN_MAXLINE + BUFSIZE+1];    /* (!usedynamic) input buffer */
#endif    /* HAVE_ICONV */
static int inbperm = 1;                                /* (usedynamic) selects bucket for lines */
static struct line_t {
    unsigned long c;
    const char *f;
    unsigned long y;
    const unsigned char *buf, *pre;
    struct line_t *link;
} *bfunc[256], *bperm[64];                             /* (usedynamic) hash table to keep lines */
static const unsigned char *prel;                      /* (usedynamic) non-preprocessed line */
#ifdef SEA_CANARY
static char *(*ngets)(char *, int, FILE *);            /* fgets for preprocessing */
#else    /* !SEA_CANARY */
static in_inc_t *incinfo[TL_INC+1];                    /* #include list */
static lex_buf_t fnbuf = {                             /* alternative buffer set */
    NULL, TL_STR,
    NULL, TL_STR,
    NULL, 10
};
#endif    /* SEA_CANARY */


/*
 *  deallocates all lines in a bucket
 */
static void freebuf(struct line_t **bucket, int n)
{
    struct line_t *p;

    assert(bucket);

    while (--n >= 0)
        while ((p = bucket[n]) != NULL) {
            void *q = (void *)p->buf,
                 *r = (void *)p->pre;
            bucket[n] = p->link;
            MEM_FREE(q);
            MEM_FREE(r);
            MEM_FREE(p);
        }
}


/*
 *  (usedynamic) adds a line into a bucket with its line number and file name
 */
static void addline(int *pi, const unsigned char *buf)
{
    int h;
    struct line_t *p, **pp;

    assert(pi);

    h = in_cpos.g.y & (((*pi)? NELEM(bperm): NELEM(bfunc)) - 1);
    pp = (*pi)? &bperm[h]: &bfunc[h];
    if (*pi > 1)    /* following lines go into bfunc */
        *pi = 0;

#if 0    /* check for uniqueness disabled for performance */
    for (p = *pp; p; p = p->link)
        if (p->f == in_cpos.g.f && p->y == in_cpos.g.y && p->c == in_cpos.g.c)
            err_issue(ERR_PP_NOUNIQUELINE);
#endif    /* disabled */

    MEM_NEW(p);
    p->c = in_cpos.g.c;
    p->f = in_cpos.g.f;
    p->y = in_cpos.g.y;
    p->buf = buf;
    p->link = *pp;
    if (prel) {
        p->pre = prel;
        prel = NULL;
    } else
        p->pre = NULL;
    *pp = p;
}


/*
 *  deallocates lines in all buckets
 */
static void freeline(void)
{
    freebuf(bfunc, NELEM(bfunc));
    freebuf(bperm, NELEM(bperm));
}


/*
 *  sets the flag to indicate following lines are in a function
 */
void (in_enterfunc)(void)
{
    assert(inbperm);

#ifdef SEA_CANARY
    inbperm = 0;
#else    /* !SEA_CANARY */
    inbperm = 2;
#endif    /* SEA_CANARY */
}


/*
 *  deallocates lines that appeared in a function
 */
void (in_exitfunc)(void)
{
    assert(inbperm != 1);

    freebuf(bfunc, NELEM(bfunc));
    inbperm = 1;
}


/*
 *  (usedynamic) retrieves a line with a line number and file name
 */
const unsigned char *(in_getline)(unsigned long cnt, const char *f, unsigned long y,
                                  const unsigned char **pre)
{
    static struct line_t **barr[] = { bfunc, bperm };
    static int size[] = { NELEM(bfunc), NELEM(bperm) };

    int i, h;
    struct line_t *p;

    if (f == in_cpos.g.f && y == in_cpos.g.y && cnt == in_cpos.g.c
#ifdef HAVE_ICONV
        && in_line != buf    /* avoids passing incompletely read line */
#endif    /* HAVE_ICONV */
       ) {
        if (pre) {
            *pre = prel;
            prel = NULL;    /* freed by issue() in err.c */
        }
        return in_line;
    }

    for (i = 0; i < NELEM(barr); i++) {
        h = y & (size[i] - 1);
        for (p = barr[i][h]; p; p = p->link)
            if (p->f == f && p->y == y && p->c == cnt) {
                if (pre) {
                    *pre = p->pre;
                    p->pre = NULL;    /* freed by issue() in err.c */
                }
                return p->buf;
            }
    }

    return NULL;
}


#ifdef HAVE_ICONV
/*
 *  counts the number of characters in UTF-8 strings;
 *  ASSUMPTION: (HAVE_ICONV) UTF-8 used as internal pivot encoding
 */
unsigned long (in_cntchar)(const unsigned char *p, const unsigned char *q)
{
    unsigned long n = 0;

    for (; p < q; p++)
        if (FIRSTUTF8(*p))
            n++;

    return n;
}
#endif    /* HAVE_ICONV */


#ifndef SEA_CANARY
/*
 *  wrapper for lex_scon() to avoid a reentrancy problem
 */
static const char *fname(void)
{
    int w = 0;
    const char *s;
    unsigned long m;

    lex_bp = &fnbuf;
    m = lex_scon(*in_cp, &w, 1);
    assert(!w);
    s = hash_new((char *)fnbuf.b.p, m);
    lex_bp = &lex_buf;

    return s;
}


/*
 *  accepts #;
 *  in_cp points to # before the call and a whole line consumed;
 *  ASSUMPTION: ULONG_MAX is not less than the max line number
 */
static void resync(void)
{
    int ovf;
    unsigned long n;

    assert(in_limit);
    assert(in_cp);

    in_cp++;    /* skips # */
    IN_SKIPSP(in_cp);
    if (in_limit - in_cp < IN_MAXLINE)
        in_fillbuf();
    if (isdigit(*in_cp)) {    /* # digits */
        in_cpos.g.c++;    /* for unique source coordinate */
        n = 0;
        ovf = 0;
        do {
            if (*in_cp == '\n')
                IN_FILLBREAK(in_cp) /* ; */
            if (n > (ULONG_MAX-(*in_cp-'0')) / 10 || n > (TL_LINENO_STD-(*in_cp-'0')) / 10)
                ovf = 1;
            n = 10*n + (*in_cp++ - '0');
        } while(ISCH_DN(*in_cp));
        if (ovf)
            in_cp--, err_issue(ERR_PP_LARGELINE), in_cp++;
        else if (n == 0)
            in_cp--, err_issue(ERR_PP_ZEROLINE), in_cp++;
        if (n == 0)    /* overflow also can result in 0 */
            n = 1;
        IN_SKIPSP(in_cp);
        if (*in_cp == '"') {    /* # digits "filename" */
            const char *s = fname();
            IN_SKIPSP(in_cp);
            switch(*in_cp) {    /* # digits "filename" code */
                case '3':
                    in_cpos.g.fl.w = 1, err_wmute();
                    /* no break */
                case '1':
                    if (in_incp > &incinfo[0]) {
                        in_inc_t *p;
                        (*in_incp)->printed = 0;
                        in_incp--;
                        if (!*in_incp)
                            MEM_NEW(*in_incp);
                        p = *in_incp;
                        p->f = in_cpos.g.f;
                        p->y = in_cpos.g.y;
                        p->printed = 0;
                    } else {
                        err_issue(ERR_PP_INCBROKEN);
                        in_cpos.g.fl.w = 0, err_wunmute();
                    }
                    in_cp++;
                    break;
                case '4':
                    in_cpos.g.fl.w = 0, err_wunmute();
                    /* no break */
                case '2':
                    assert(s);
                    if (in_incp < &incinfo[NELEM(incinfo)-1]) {
                        if ((*in_incp)->f != s) {
                            err_issue(ERR_PP_INCNOTENTER, s);
                            s = (char *)(*in_incp)->f;
                        }
                        in_incp++;
                    } else {
                        err_issue(ERR_PP_INCNOTENTER, s);
                        err_issue(ERR_PP_INCBROKEN);
                    }
                    in_cp++;
                    break;
            }
            IN_SKIPSP(in_cp);
            if (*in_cp != '\n' && !main_opt()->extension)
                err_issue(ERR_LEX_EXTRATOKEN);
            in_cpos.g.y = n - 1;
            in_cpos.g.f = s;
            if ((*in_incp)->f)
                in_cpos.g.fl.n = 0;
            else {
                in_cpos.g.fl.n = 1;
                in_cpos.g.fy = in_cpos.g.y;
            }
        } else {
            if (*in_cp != '\n' && !main_opt()->extension)
                err_issue(ERR_LEX_EXTRATOKEN);
            if (main_opt()->diagstyle != 2)
                in_cpos.g.y = n - 1;
        }
        IN_DISCARD(in_cp);
        return;
    }

    err_issue(ERR_PP_UNKNOWNDIRW);
    IN_DISCARD(in_cp);
}
#endif    /* !SEA_CANARY */


/*
 *  (usedynamic) replaces nextlined after EOF seen;
 *  used to avoid passing buf to addline() and incrementing in_cpos.g.[f]y
 */
static void eofd(void)
{
    in_limit = in_cp;
    return;
}


#ifdef SEA_CANARY
#define getct(fp) ((nq > 0)? (nq--, '?'): getc(fp))

/*
 *  mimics fgets() handling trigraphs
 */
static char *fgetst(char *buf, int n, FILE *fp)
{
    static int nq;

    int c = 0;
    unsigned char *s;

    assert(buf);
    assert(fp);

    if (n <= 1)
        return NULL;
    s = (unsigned char *)buf;

    while (n > 0 && c != '\n') {
        c = getct(fp);
        if (c == EOF)
            break;
        if (c == '?') {
            if ((c = getct(fp)) == '?') {    /* ?? */
                switch(c = getct(fp)) {
                    case '(':    /* ??( */
                        c = '[';
                        break;
                    case ')':    /* ??) */
                        c = ']';
                        break;
                    case '<':    /* ??< */
                        c = '{';
                        break;
                    case '>':    /* ??> */
                        c = '}';
                        break;
                    case '=':    /* ??= */
                        c = '#';
                        break;
                    case '/':    /* ??/ */
                        c = '\\';
                        break;
                    case '\'':    /* ??' */
                        c = '^';
                        break;
                    case '!':    /* ??! */
                        c = '|';
                        break;
                    case '-':    /* ??- */
                        c = '~';
                        break;
                    case '?':    /* ??? */
                        c = '?';
                        nq += 2;
                        break;
                    default:    /* ?? + others */
                        nq++;
                        ungetc(c, fp);
                        c = '?';
                        break;
                }
            } else {    /* ? + others */
                ungetc(c, fp);
                c = '?';
            }
        }
        *s++ = c;
        n--;
    }
    if (s == (unsigned char *)buf)
        return NULL;
    else {
        *s = '\0';
        return buf;
    }
}
#endif    /* SEA_CANARY */


/*
 *  (usedynamic) reads the next line;
 *  dynamic allocation version;
 *  in_limit points to one past the terminating null unless EOF;
 *  ASSUMPTION: '\n' is no part of multibyte characters and has no effect on the shift state;
 *  ASSUMPTION: charset in which source is written is same as that in which beluga is running
 */
static void nextlined(void)
{
#ifdef HAVE_ICONV
    static unsigned long buflen = BUFLEN;
#endif    /* HAVE_ICONV */

    unsigned char *p;
    unsigned long len;
#ifndef HAVE_ICONV
    unsigned long buflen;
#endif    /* !HAVE_ICONV */
#ifdef SEA_CANARY
    unsigned long y = 0;
#endif    /* SEA_CANARY */

    assert(in_nextline == nextlined);
    assert(in_limit);
    assert(in_line);
    assert(in_cp);
    assert(fptr);

    /* could be done with len below before line consumed,
       but done after consumed to match to nextlines();
       note TL_LINE_STD is of unsigned long */
#ifdef HAVE_ICONV
    if ((main_iton && in_cntchar(in_line, in_limit) > TL_LINE_STD + 1) ||
        (!main_iton && in_limit-in_line > TL_LINE_STD + 1)) {
#else    /* !HAVE_ICONV */
    if (in_limit-in_line > TL_LINE_STD + 1) {
#endif    /* HAVE_ICONV */
        in_cp--;
        err_issue(ERR_INPUT_LONGLINE);
        err_issue(ERR_INPUT_LONGLINESTD, (unsigned long)TL_LINE_STD);
        in_cp++;
    }
    addline(&inbperm, in_line);
#ifdef HAVE_ICONV
    if (main_iton)
        p = (unsigned char *)(in_limit = in_line = in_cp = buf);
    else
#endif    /* HAVE_ICONV */
        p = (unsigned char *)(in_limit = in_line = in_cp = MEM_ALLOC(buflen=BUFLEN));
    *p = '\0';
    len = 0;

    while (1) {
        assert(buflen-len > 1);
        /* retrun value of fgets() need not be checked;
           *p is NUL and len is 0 for start of each line */
#ifdef SEA_CANARY
        ngets((char *)p+len, buflen-len, fptr);
#else    /* !SEA_CANARY */
        fgets((char *)p+len, buflen-len, fptr);
#endif    /* SEA_CANARY */
        if (ferror(fptr)) {
            err_issue(ERR_INPUT_ERROR);
            in_nextline = eofd;
            break;
        }
        len += strlen((char *)p+len);
        if (len == 0) {    /* real EOF */
            in_cpos.g.y++;
            in_cpos.g.fy += in_cpos.g.fl.n;
#ifdef SEA_CANARY
            in_cpos.my++;
#endif
            in_nextline = eofd;
            break;
        }
#ifdef SEA_CANARY
        if (len > 1 && p[len-2] == '\\' && p[len-1] == '\n') {
            int c = getc(fptr);
            y++;
            if (c == EOF) {
                err_issue(ERR_INPUT_BSNLEOF);
                p[len-2] = '\n';
                p[--len] = '\0';
            } else {
                ungetc(c, fptr);
                p[len -= 2] = '\0';
            }
            continue;
        }
#endif    /* SEA_CANARY */
        if (p[len-1] == '\n' || feof(fptr)) {    /* line completed */
#ifdef SEA_CANARY
            in_cpos.g.y += (y + 1);
            if (in_cpos.g.fl.n)
                in_cpos.g.fy += (y + 1);
            in_cpos.my += (y + 1);
#else    /* !SEA_CANARY */
            in_cpos.g.y++;
            in_cpos.g.fy += in_cpos.g.fl.n;
#endif    /* SEA_CANARY */
#ifdef HAVE_ICONV
            if (main_iton) {
                ICONV_DECL((char *)p, len + 1);    /* +1 to include NUL */
                olenv = olen = (ilenv > BUFLEN)? ilenv: BUFLEN;
                obufv = obuf = MEM_ALLOC(olenv);
                ICONV_DO(main_iton, 0, { pos.g = in_cpos.g;
                                         pos.x = (ibufv - (char *)p + 1);
                                         err_issuep(&pos, ERR_INPUT_CONVFAIL); });
                p = (unsigned char *)obuf;
                len = olen - olenv - 1;
            }
#endif    /* HAVE_ICONV */
            in_line = p;
            if (p[len-1] != '\n') {
                p[len++] = '\n';
                p[len] = '\0';
                if (main_opt()->std)
                    in_cp = &p[len-1];
                err_issue(ERR_INPUT_NOTENDNL);
            }
            in_limit = &p[len+1];
            in_cp = p;
#ifndef SEA_CANARY
            if (main_opt()->_internal && in_cp[0] == '@') {
                char *p = MEM_ALLOC(in_limit - in_line);    /* in_limit points to one past null */
                prel = (unsigned char *)strcpy(p, (char *)in_cp+1);
                in_cpos.g.fy -= in_cpos.g.fl.n;
                in_cpos.g.y--;
                len = 0;
                continue;
            }
            IN_SKIPSP(in_cp);
            if (*in_cp == '\n')    /* whole line consumed */
                nextlined();
            else if (in_cp[0] == '#' && in_cp[1] != '#') {    /* # or #line expected */
                resync();
                nextlined();
            }
#endif    /* !SEA_CANARY */
            return;
        } else {    /* expands buffer */
            MEM_RESIZE(p, buflen+=BUFLEN);
#ifdef HAVE_ICONV
            if (main_iton)    /* as if nextlined() just started */
                in_limit = in_line = in_cp = buf = p;
#endif    /* HAVE_ICONV */
        }
    }
    /* EOF */
    p[0] = '\n';
    p[1] = '\0';
    in_limit = in_cp = in_line;
}


/*
 *  (!usedynamic) reads the next line;
 *  static allocation version;
 *  ASSUMPTION: charset in which source is written is same as that in which beluga is running
 */
static void nextlines(void)
{
#ifdef SEA_CANARY
    assert(!"impossible control flow -- should never reach here");
#endif    /* SEA_CANARY */
    assert(in_nextline == nextlines);
    assert(in_cp);
    assert(in_limit);

    if (in_cp > in_limit)
        in_fillbuf();
    else {
        if (in_cp-in_line+in_outlen > TL_LINE_STD) {
            in_cp--;
            err_issue(ERR_INPUT_LONGLINE);
            err_issue(ERR_INPUT_LONGLINESTD, (unsigned long)TL_LINE_STD);
            in_cp++;
        }
        if (in_cp == in_limit && bsize == 0)
            return;
        in_cpos.g.y++;
        in_cpos.g.fy += in_cpos.g.fl.n;
        in_outlen = 0;
        in_line = in_cp;
#ifndef SEA_CANARY
        if (main_opt()->_internal && in_cp[0] == '@') {
            IN_DISCARD(in_cp);
            in_cpos.g.fy -= in_cpos.g.fl.n;
            in_cpos.g.y--;
            nextlines();
        } else {
            IN_SKIPSP(in_cp);
            if (*in_cp == '#') {
                resync();
                nextlines();
            }
        }
#endif    /* SEA_CANARY */
    }
}


/*
 *  prepares to accept input;
 *  also called when an input file changed
 */
void (in_init)(FILE *fp, const char *fn)
{
#ifndef SEA_CANARY
    static in_inc_t sentinel;
#endif    /* !SEA_CANARY */
#ifdef HAVE_ICONV
    static unsigned char ib[1+IN_MAXLINE + BUFSIZE+1];    /* !usedynamic && !main_iton */
#endif    /* HAVE_ICONV */

    assert(fp);
#ifndef SEA_CANARY
    assert(!fnbuf.b.p);
    assert(!fnbuf.s.p);
    assert(!fnbuf.t.p);
#endif    /* !SEA_CANARY */

    fptr = fp;
    if (!fn || *fn == '\0')
        fn = "<stdin>";
    in_cpos.ff = in_cpos.g.f = hash_string(fn);
    in_cpos.g.fl.n = 1;
#ifdef HAVE_ICONV
    if (usedynamic || main_iton) {
#else    /* !HAVE_ICONV */
    if (usedynamic) {
#endif    /* HAVE_ICONV */
        assert(BUFLEN > 1);
        in_limit = in_line = in_cp = MEM_ALLOC(2);
#ifdef HAVE_ICONV
        if (main_iton)
            buf = MEM_ALLOC(BUFLEN);
#endif    /* HAVE_ICONV */
#ifdef SEA_CANARY
        ngets = (main_opt()->trigraph)? fgetst: fgets;
#endif    /* SEA_CANARY */
        in_nextline = nextlined;
        bsize = 0;
    } else {
        assert(IN_MAXTOKEN >= 32);
        assert(IN_MAXLINE > IN_MAXTOKEN);
        assert(BUFSIZE > IN_MAXLINE);
#ifdef HAVE_ICONV
        buf = ib;
#endif    /* HAVE_ICONV */
        in_limit = in_line = in_cp = &buf[IN_MAXLINE];
        in_nextline = nextlines;
        bsize = -1;
    }
#ifndef SEA_CANARY
    incinfo[NELEM(incinfo)-1] = &sentinel;
    in_incp = &incinfo[NELEM(incinfo)-1];

    fnbuf.b.p = MEM_ALLOC(fnbuf.b.n);
    fnbuf.s.p = MEM_ALLOC(fnbuf.s.n + 1);
    fnbuf.t.p = MEM_ALLOC(fnbuf.t.n * sizeof(*fnbuf.t.p));
#endif    /* !SEA_CANARY */

    in_fillbuf();
    in_nextline();
}


/*
 *  (!usedynamic) fills the input buffer;
 *  never called when in_cp-in_limit > IN_MAXLINE
 */
void (in_fillbuf)(void)
{
    assert(in_cp);
    assert(in_limit);

    if (bsize == 0) {
        if (in_cp > in_limit)
            in_cp = in_limit;
        return;
    }

    assert(in_nextline == nextlines);
    assert(in_line);
    assert(fptr);

    if (in_cp >= in_limit) {
        assert(in_limit - in_line >= 0);
        in_outlen += in_limit - in_line;
        in_line = in_cp = &buf[IN_MAXLINE];
    } else {
        int n = in_limit - in_cp;
        unsigned char *s;
        assert(n <= IN_MAXLINE);
        assert(in_cp - in_line >= 0);
        in_outlen += in_cp - in_line;
        s = &buf[IN_MAXLINE] - n;
        in_line = s;
        while (in_cp < in_limit)
            *s++ = *in_cp++;
        in_cp = &buf[IN_MAXLINE] - n;
    }

    if (feof(fptr)) {
        if (in_limit[-1] != '\n')
            err_issue(ERR_INPUT_NOTENDNL);
        bsize = 0;
    } else
        bsize = fread(&buf[IN_MAXLINE], 1, BUFSIZE, fptr);
    if (ferror(fptr)) {    /* makes as if EOF */
        err_issue(ERR_INPUT_ERROR);
        bsize = 0;
        in_cp = &buf[IN_MAXLINE+bsize];
    }
    in_limit = &buf[IN_MAXLINE+bsize];
    *(unsigned char *)in_limit = '\n';
}


#ifdef SEA_CANARY
/*
 *  saves or restores input file context
 */
void (in_switch)(FILE *fp, const char *fn)
{
    in_nextline = nextlined;
    if (fp) {    /* push */
        inc_push(fptr);
        fptr = fp;
        if (in_cpos.g.fl.n)
            in_cpos.g.fy--;    /* newline on #include already seen */
        in_cpos.g.f = hash_string(fn);
        in_cpos.my = in_cpos.g.fl.n = in_cpos.g.y = 0;
        in_cpos.mf = NULL;
        /* usedynamic is always true and see in_init() */
        in_limit = in_line = in_cp = MEM_ALLOC(2);
        in_nextline();
    } else {    /* pop */
#ifdef HAVE_ICONV
        if (!main_iton)
#endif    /* HAVE_ICONV */
        {
            void *p = (void *)in_line;
            MEM_FREE(p);
        }
        fptr = inc_pop(fptr);
        if ((in_cpos.g.fl.n=inc_isffile()) == 1)
            in_cpos.g.fy++;    /* newline on #include already seen */
    }
}


/*
 *  remembers the current line for diagnostics
 */
void (in_toperm)(void)
{
    int f = 2;
    char *p = MEM_ALLOC(in_limit - in_line);    /* in_limit points to one past null */

    addline(&f, (unsigned char *)strcpy(p, (char *)in_line));
}
#endif    /* SEA_CANARY */


/*
 *  finalizes input
 */
void (in_close)(void)
{
#ifndef SEA_CANARY
    in_inc_t **p;

    assert(in_incp);
    assert(fnbuf.b.p);
    assert(fnbuf.s.p);
    assert(fnbuf.t.p);
#endif    /* !SEA_CANARY */

    if (in_nextline == eofd || in_nextline == nextlined) {
        void *q = (void *)in_line;
#ifdef HAVE_ICONV
        if (in_line != buf)
            MEM_FREE(buf);
#endif    /* HAVE_ICONV */
        MEM_FREE(q);
        in_limit = in_line = in_cp = NULL;
        freeline();
    }

#ifndef SEA_CANARY
    if (err_count() == 0 && in_incp < &incinfo[NELEM(incinfo)-1]) {
        err_issue(ERR_PP_INCNOTLEAVE, in_cpos.g.f);
        for (p = in_incp+1; p < &incinfo[NELEM(incinfo)-1]; p++)
            err_issue(ERR_PP_INCNOTLEAVE, p[-1]->f);
    }
    for (p = incinfo; p < &incinfo[NELEM(incinfo)-1]; p++)
        if (*p)
            MEM_FREE(*p);

    MEM_FREE(fnbuf.b.p);
    MEM_FREE(fnbuf.s.p);
    MEM_FREE(fnbuf.t.p);
#endif    /* !SEA_CANARY */
}

/* end of in.c */
