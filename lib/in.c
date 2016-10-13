/*
 *  input handling
 */

#include <stddef.h>        /* NULL */
#include <stdio.h>         /* FILE, fgets, ferror, feof */
#include <string.h>        /* strlen */
#include <cbl/assert.h>    /* assert */
#include <cbl/memory.h>    /* MEM_ALLOC, MEM_RESIZE, MEM_FREE */
#include <cdsl/hash.h>     /* hash_string */

#include "common.h"
#include "err.h"
#include "lmap.h"
#include "main.h"
#include "util.h"
#include "in.h"


sz_t in_py;                   /* physical line # of current file */
const char *in_line;          /* beginning of current line */
const char *in_cp;            /* current character */
const char *in_limit;         /* end of current line */
void (*in_nextline)(void);    /* function to read next input line */


static FILE *fptr;    /* file pointer for input */
static char *buf;     /* input buffer */
static sz_t bufn;     /* input buffer size */
#ifdef HAVE_ICONV
static char *ibuf;    /* UTF-8 input buffer */
static sz_t ibufn;    /* UTF-8 input buffer size */
#endif    /* HAVE_ICONV */


/*
 *  converts a trigraph warning its first use
 */
int (in_trigraph)(const char *p)
{
    int c;

    assert(p);

    switch(p[2]) {
        case '(':
            c = '[';
            break;
        case ')':
            c = ']';
            break;
        case '<':
            c = '{';
            break;
        case '>':
            c = '}';
            break;
        case '=':
            c = '#';
            break;
        case '/':
            c = '\\';
            break;
        case '\'':
            c = '^';
            break;
        case '!':
            c = '|';
            break;
        case '-':
            c = '~';
            break;
        default:
            return '?';
    }

    if (main_opt()->trigraph >= 2)
        err_dline(p, 3, (main_opt()->trigraph & 1)?
                            ERR_INPUT_TRIGRAPH: ERR_INPUT_TRIGRAPHI, p[2], c);
    return c;
}


/*
 *  counts the number of characters in UTF-8 strings;
 *  ASSUMPTION: UTF-8 used as default and (HAVE_ICONV) internal pivot encoding
 */
sz_t (in_cntchar)(const char *p, const char *q, sz_t m, const char **pp)
{
    sz_t n = 0;

    for (; (!q || p < q) && (m == (sz_t)-1 || n < m); p++) {
        if (*p == '\n')
            continue;
        if (FIRSTUTF8(*p))
            n++;
        if (p[0] == '?' && p[1] == '?' && (main_opt()->trigraph & 1)) {
            switch(p[2]) {
                case '(':    /* [ */
                case ')':    /* ] */
                case '<':    /* { */
                case '>':    /* } */
                case '=':    /* # */
                case '/':    /* \ */
                case '\'':    /* ^ */
                case '!':    /* | */
                case '-':    /* ~ */
                    p += 2;
                    break;
            }
        }
    }
    while (!FIRSTUTF8(*p) && (!q || p < q))
        p++;

    if (pp)
        *pp = p;    /* q or points to char after m chars */
    return n;
}


/*
 *  replaces nextline() after EOF seen
 */
static void eof(void)
{
    in_limit = in_cp;
    return;
}


/*
 *  reads the next line;
 *  in_limit points to one past the terminating null unless EOF;
 *  ASSUMPTION: '\n' is no part of multibyte characters and has no effect on the shift state;
 *  ASSUMPTION: charset in which source is written is same as that in which beluga is running
 */
static void nextline(void)
{
    static int bs = 0;

    char *p;
    sz_t len;

    assert(fptr);

    if (bs > 0)
        in_py += bs, bs = 0;
    lmap_fline(++in_py, ftell(fptr));
    p = (char *)(in_limit = in_line = in_cp = buf);
    *p = '\0';
    len = 0;

    while (1) {
        assert(bufn-len > 1);
        /* retrun value of fgets() need not be checked;
           *p is NUL and len is 0 for start of each line */
        fgets((char *)p+len, bufn-len, fptr);
        if (ferror(fptr)) {
            err_dline(NULL, 1, ERR_INPUT_ERROR);
            in_nextline = eof;
            break;
        }
        len += strlen((char *)p+len);
        if (len == 0) {    /* real EOF */
            in_nextline = eof;
            break;
        }
        if (len > 1 && (p[len-2] == '\\' ||
                        (main_opt()->trigraph && len > 3 &&
                         p[len-4] == '?' && p[len-3] == '?' && p[len-2] == '/')) &&
            p[len-1] == '\n') {
            if (p[len-2] == '/')
                in_trigraph(&p[len-4]);    /* for warning */
            if (p[len-2] == '\\' || (main_opt()->trigraph & 1)) {    /* line splicing */
                int n = 1+1;
                int c;
                lmap_fline(in_py + ++bs, ftell(fptr));
                c = getc(fptr);
                if (p[len-2] == '/')
                    len -= 2, n = 3+1;
                if (c == EOF) {
                    err_dline(p+len-2, n, ERR_INPUT_BSNLEOF);
                    p[len-2] = '\n';
                    p[--len] = '\0';
                    bs--;    /* for better tracking of locus */
                } else {
                    ungetc(c, fptr);
                    p[--len-1] = '\n';
                    continue;
                }
            }
        }
        if (p[len-1] == '\n' || feof(fptr)) {    /* line completed */
#ifdef HAVE_ICONV
            if (main_iton) {
                ICONV_DECL((char *)p, len + 1);    /* +1 to include NUL */
                olenv = olen = ibufn;
                obufv = obuf = ibuf;
                ICONV_DO(main_iton, 0, {});
                ibuf = p = obuf;
                len = olen - olenv - 1;
            }
#endif    /* HAVE_ICONV */
            in_line = p;
            if (!feof(fptr))    /* newline read from input */
                p[--len] = '\0';
            else if (p[len-1] != '\n')    /* EOF without newline */
                err_dline(p+len, 1, ERR_INPUT_NOTENDNL);
            in_limit = &p[len+1];
            in_cp = p;
            if (main_opt()->std) {
                const char *q;
                sz_t c = in_cntchar(p, &p[len], TL_LINE_STD, &q);
                if (c >= TL_LINE_STD) {
                    err_dline(q, 1, ERR_INPUT_LONGLINE);
                    err_dline(NULL, 1, ERR_INPUT_LONGLINESTD, (unsigned long)TL_LINE_STD);
                }
            }
            return;
        } else {    /* expands buffer */
            MEM_RESIZE(p, bufn+=BUFUNIT);
            buf = p;
        }
    }
    /* EOF */
    p[0] = '\0';
    in_limit = in_cp = in_line;
}


/*
 *  prepares to accept input;
 *  also called when an input file changed
 */
void (in_init)(FILE *fp, const char *fn)
{
    const char *rfn;

    assert(fp);

    fptr = fp;
    if (fn)
        rfn = rpath(fn);
    else {    /* stdin */
        fn = "";
        rfn = hash_string(fn);
    }
    lmap_init(hash_string(fn), rfn);
    lmap_flset(rfn);

    assert(BUFUNIT > 1);
    buf = MEM_ALLOC(bufn = BUFUNIT);
#ifdef HAVE_ICONV
    if (main_iton)
        ibuf = MEM_ALLOC(ibufn = BUFUNIT);
#endif    /* HAVE_ICONV */
    in_nextline = nextline;
    nextline();
}


/*
 *  finalizes input
 */
void (in_close)(void)
{
    MEM_FREE(buf);
#ifdef HAVE_ICONV
    if (main_iton)
        MEM_FREE(ibuf);
#endif    /* HAVE_ICONV */
    in_limit = in_line = in_cp = NULL;

    lmap_close();
}


/*
 *  counts characters with wcwidth();
 *  ASSUMPTION: the result is less than max of sz_t;
 *  ASSUMPTION: invalid byte occupies single column;
 *  ASSUMPTION: UTF-8 used as default and (HAVE_ICONV) internal pivot encoding
 */
sz_t (in_getwx)(sz_t wx, const char *s, const char *p, int *pdy)
{
    int dy = 0;
    unsigned long wc;

    assert(s);
    assert(p);

    while (s < p) {
        if (*s == '\n') {
            s++;
            dy++;
            wx = 1;
            continue;
        }
        wc = utf8to32(&s);
        wx += (wc == (unsigned long)-1)? 1: wcwidth(wc);
    }

    if (pdy)
        *pdy = dy;
    return wx;
}

/* end of in.c */
