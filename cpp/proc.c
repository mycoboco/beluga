/*
 *  processing for preprocessor
 */

#include <ctype.h>         /* isdigit, isxdigit */
#include <limits.h>        /* ULONG_MAX */
#include <stddef.h>        /* size_t, NULL */
#include <stdio.h>         /* FILE, fprintf, putc, fputs */
#include <string.h>        /* strlen, strcpy */
#include <cbl/arena.h>     /* ARENA_FREE */
#include <cbl/assert.h>    /* assert */
#include <cbl/memory.h>    /* MEM_ALLOC, MEM_RESIZE, MEM_FREE */
#include <cdsl/hash.h>     /* hash_string, hash_new */

#include "../src/common.h"
#include "../src/err.h"
#include "../src/in.h"
#include "../src/main.h"
#include "cond.h"
#include "ctx.h"
#include "expr.h"
#include "inc.h"
#include "lex.h"
#include "lxl.h"
#include "mcr.h"
#include "mg.h"
#include "prgm.h"
#include "strg.h"
#include "util.h"
#include "proc.h"

#define setout(p) (ctx_cur->list->head = out = (p))    /* adjusts output pointer */
#define reset()   (setout(ctx_cur->cur))               /* moves output pointer forward without
                                                          emitting tokens */
#define T(p)      ((lex_t *)(p))                       /* shorthand for cast to lex_t * */

/* gives proper locus after macro expansion */
#define PPOS(p) ((mcr_mpos)? mcr_mpos: ((p)->y > 0)? (p): &lex_cpos)


/* processing states */
enum {
    SINIT,      /* initial or after macro guard */
    SIDIREC,    /* directive after initial */
    SAFTRNL,    /* after newline */
    SDIREC,     /* directive */
    SNORM,      /* normal */
    SIGN        /* ignore */
};

/* directives */
enum {
    DINCLUDE,    /* #include */
    DDEFINE,     /* #define */
    DUNDEF,      /* #undef */
    DIF,         /* #if */
    DIFDEF,      /* #ifdef */
    DIFNDEF,     /* #ifndef */
    DELIF,       /* #elif */
    DELSE,       /* #else */
    DENDIF,      /* #endif */
    DLINE,       /* #line */
    DERROR,      /* #error */
    DPRAGMA      /* #pragma */
};


/* internal functions referenced forwardly */
static void outtokn(const lex_t *);
static void outtokp(const lex_t *);
static lex_t *direci(lex_t *);


/* diagnoses extra tokens if set */
static int warnxtra[] = {
    0,    /* #include */
    0,    /* #define */
    1,    /* #undef */
    0,    /* #if */
    1,    /* #ifdef */
    1,    /* #ifndef */
    0,    /* #elif */
    1,    /* #else */
    1,    /* #endif */
    0,    /* #line; check done in dline() */
    0,    /* #error */
    1,    /* #pragma */
    0     /* unknown */
};

static void (*outtok)(const lex_t *) = outtokn;    /* prints a token */
static lex_t *(*directive)(lex_t *) = direci;      /* handles "directive" state */

/* directive look-up table */
static struct {
    const char *name;
    int code;
} dtab[] = {
    "include", DINCLUDE,
    "define",  DDEFINE,
    "undef",   DUNDEF,
    "if",      DIF,
    "ifdef",   DIFDEF,
    "ifndef",  DIFNDEF,
    "elif",    DELIF,
    "else",    DELSE,
    "endif",   DENDIF,
    "line",    DLINE,
    "error",   DERROR,
    "pragma",  DPRAGMA
};

/* 0 if no spaces are necessary;
   1 if space is necessary after the token;
   2 if space is necessary before the token and
   3 if spaces are necessary before and after the token */
static char toksp[] = {
#define xx(a, b, c, d, e, f, g, h) h,
#define yy(a, b, c, d, e, f, g, h) h,
#include "../src/xtoken.h"
};

static FILE *outfile;           /* output file */
static int ptid;                /* id of recently printed token */
static unsigned long ty = 1;    /* line number of token printed last */
static int state = SINIT;       /* current state */
static lxl_node_t *out;         /* output pointer */


/*
 *  skips tokens until encountering a newline exclusive
 */
static lex_t *skiptonl(lex_t *t)
{
    assert(t);

    while (t->id != LEX_NEWLINE) {
        assert(t->id != LEX_EOI);
        t = lxl_next();
    }
    reset();

    return t;
}


/*
 *  skips whitespaces;
 *  skip() from util.c not used for performance and a call to reset()
 */
static lex_t *skipsp(lex_t *t)
{
    assert(t);

    while (t->id == LEX_SPACE)
        t = lxl_next();
    reset();

    return t;
}


/*
 *  prints a string escaping special characters
 */
static void printesc(const char *s)
{
    assert(s);

    for (; *s; s++)
        switch(*s) {
            case '\\':
                fputs("\\\\", outfile);
                break;
            case '"':
                fputs("\\\"", outfile);
                break;
            /* others are printed without escaping */
            case '\a':
            case '\b':
            case '\f':
            case '\n':
            case '\r':
            case '\t':
            default:
                putc(*s, outfile);
                break;
        }
}


/*
 *  prints a locus with #
 */
static void outpos(unsigned long y, const char *file, int stk)
{
    static struct {
        unsigned long y;
        const char *f;
    } pos;

    switch(stk) {
        case 0:    /* set */
            if (y == ty && file == pos.f)
                break;
            ty = pos.y = y;
            pos.f = file;
            outtok = outtokp;
            break;
        default:
            assert(stk < 0);
            if (!file) {    /* print saved */
                y = pos.y;
                file = pos.f;
            }
            goto print;
        case 1:    /* set and print given */
        case 2:
            pos.y = y;
            pos.f = file;
        print:
            fprintf(outfile, "# %lu \"", y);
            printesc(file);
            if (stk > 0)
                fprintf(outfile, "\" %d", stk);
            else
                putc('"', outfile);
            putc('\n', outfile);
            ty = y;
            outtok = outtokn;
            break;
    }
}


/*
 *  prints a token when line sync is necessary
 */
static void outtokp(const lex_t *t)
{
    const lex_pos_t *ppos;

    assert(t);

    switch(t->id) {
        case LEX_NEWLINE:
            ppos = (lex_pos_t *)t->rep;
            outpos(ppos->y+1, ppos->f, 0);
            break;
        case LEX_EOI:
            assert(!"invalid token -- should never reach here");
            break;
        default:
            outpos(0, NULL, -1);
            fputs(t->rep, outfile);
            break;
    }
}


/*
 *  prints a token when line sync is unnecessary
 */
static void outtokn(const lex_t *t)
{
    const lex_pos_t *ppos;

    assert(t);

    switch(t->id) {
        case LEX_NEWLINE:
            ppos = (lex_pos_t *)t->rep;
            putc('\n', outfile);
            if (ppos->y != ty)
                outpos(ppos->y+1, ppos->f, 0);
            else
                ty++;
            break;
        case LEX_EOI:
            assert(!"invalid token -- should never reach here");
            break;
        default:
            fputs(t->rep, outfile);
            break;
    }
}


/*
 *  prints tokens until the current node of the context
 */
static void flush(void)
{
    static int needsp;    /* true if space needed before the current token */

    assert(ctx_isbase());

    while (out != ctx_cur->cur) {
        switch(out->kind) {
            case LXL_KSTART:
            case LXL_KEND:
                needsp = toksp[ptid] & 1;
                break;
            case LXL_KTOK:
                if (needsp) {
                    if (toksp[out->u.t.tok->id] & 2)
                        putc(' ', outfile);
                    needsp = 0;
                }
                outtok(out->u.t.tok);
                ptid = out->u.t.tok->id;
                break;
            case LXL_KHEAD:
            case LXL_KTOKI:
                break;
            case LXL_KEOL:
            default:
                assert(!"invalid node kind -- should never reach here");
                break;
        }
        setout(out->next);
    }
    if (ctx_cur->cur->strgno >= 0)
        strg_freel(ctx_cur->cur->strgno);
}


/*
 *  handles the "after newline" state
 */
static lex_t *aftrnl(lex_t *t)
{
    assert(t);

    while (1) {
        flush();
        while (t->id == LEX_SPACE)
            t = lxl_next();
        switch(t->id) {
            case LEX_NEWLINE:
                t = lxl_next();
                continue;
            case LEX_SHARP:
                state++;
                assert(state == SIDIREC || state == SDIREC);
                lex_direc = 1;
                return lxl_next();
            default:
                state = SNORM;
                /* no break */
            case LEX_EOI:
                return t;
        }
    }

    /* assert(!"impossible control flow -- should never reach here");
       return NULL; */
}


/*
 *  accepts #include;
 *  cannot use snbuf() because of a call to inc_start()
 */
static lex_t *dinclude(void)
{
    static char buf[64+1];    /* size must be (power of 2) + 1 */

    lex_t *t;
    const char *inc = NULL;
    lex_pos_t hpos, epos = { 0, };
    char *pbuf = buf, *p;
    unsigned long y = 0;
    const char *f = NULL;

    if (outtok == outtokp) {
        y = in_cpos.y;
        f = in_cpos.f;
    }
    lex_inc = 1;
    t = skipsp(lxl_next());
    hpos = lex_cpos;
    if (t->id == LEX_HEADER) {
        inc = t->rep;
        while ((t=skipsp(lxl_next()))->id != LEX_NEWLINE) {
            assert(t->id != LEX_EOI);
            if (t->id == LEX_ID && !t->blue) {
                epos = lex_cpos;
                if (mcr_expand(t, &lex_cpos))
                    continue;
            }
            err_issuep(PPOS(&epos), ERR_PP_EXTRATOKEN);
            t = skiptonl(t);
            break;
        }
    } else {
        int st = 0;    /* initial */
        size_t blen, slen;

        while (t->id != LEX_NEWLINE) {
            assert(t->id != LEX_EOI);
            if (t->id == LEX_ID && !t->blue) {
                epos = lex_cpos;
                if (mcr_expand(t, &lex_cpos)) {
                    t = skipsp(lxl_next());
                    continue;
                }
            }
            assert(t->id != LEX_NEWLINE);
            switch(st) {
                case 0:    /* initial */
                    switch(t->id) {
                        case LEX_SCON:
                            if (t->rep[0] == 'L' || t->rep[0] == '\'') {
                                default:
                                    t = skiptonl(lxl_next());
                                    continue;
                            }
                            /* no break */
                        case LEX_HEADER:
                            st = 1;
                            inc = t->rep;
                            break;
                        case '<':
                            assert(pbuf == buf);
                            st = 2;
                            p = pbuf;
                            blen = sizeof(buf) - 1;
                            *p++ = '<';
                            break;
                    }
                    break;
                case 1:    /* extra tokens */
                    err_issuep(PPOS(&epos), ERR_PP_EXTRATOKEN);
                    t = skiptonl(lxl_next());
                    continue;
                case 2:    /* '<' seen */
                    slen = strlen(t->rep);
                    if (slen > blen - (p-pbuf)) {
                        const char *oldp = pbuf;
                        blen += ((slen + NELEM(buf)-2) & ~(size_t)(NELEM(buf)-2));
                        if (pbuf == buf) {
                            pbuf = MEM_ALLOC(blen + 1);
                            strcpy(pbuf, oldp);
                        } else
                            MEM_RESIZE(pbuf, blen + 1);
                        p = pbuf + (p - oldp);
                    }
                    strcpy(p, t->rep);
                    p += slen;
                    if (t->id == '>') {
                        st = 1;
                        inc = pbuf;
                        err_issuep(&hpos, ERR_PP_COMBINEHDR);
                    }
                    break;
                default:
                    assert(!"invalid state -- should never reach here");
                    break;
            }
            t = skipsp(lxl_next());
        }
        if (!inc)
            err_issuep(&hpos, ERR_PP_NOHEADER);
    }

    if (inc && inc_start(inc, &hpos)) {
        assert(t->id == LEX_NEWLINE);
        if (f)
            outpos(y, f, -1);    /* to locate #include in compiler */
        outpos(1, in_cpos.f, 1);
        ((lex_pos_t *)t->rep)->y = 0;    /* used before discard in direci() */
    }

    if (pbuf != buf)
        MEM_FREE(pbuf);
    lex_inc = 0;
    return t;
}


/*
 *  accepts #undef
 */
static lex_t *dundef(void)
{
    lex_t *t;

    t = skipsp(lxl_next());
    if (t->id != LEX_ID) {
        err_issuep(&lex_cpos, ERR_PP_NOMCRID);
        return skiptonl(t);
    }
    mcr_del(t->rep, &lex_cpos);

    return skipsp(lxl_next());
}


/*
 *  accepts #if, #ifdef or #ifndef (a start of conditionals);
 *  ASSUMPTION: the target has no signed zero
 */
static lex_t *dif(int kind, int ign)
{
    lex_t *t;
    expr_t *c;

    cond_push(kind, &lex_cpos);
    t = lxl_next();
    if (ign) {
        cond_list->f.ignore = 2;    /* inside ignoring section */
        return skiptonl(t);
    }
    t = skipsp(t);
    switch(kind) {
        case COND_KIF:
            if (t->id == LEX_NEWLINE)
                err_issuep(&lex_cpos, ERR_PP_NOIFEXPR, "#if");
            else {
                c = expr_start(&t, "#if");
                cond_list->f.once = !(cond_list->f.ignore = (c->u.u == 0));
            }
            break;
        case COND_KIFNDEF:
            if (mg_state == MG_SINIT && state == SIDIREC)
                mg_state = MG_SIFNDEF;
        case COND_KIFDEF:
            if (t->id != LEX_ID) {
                err_issuep(&lex_cpos, ERR_PP_NOIFID, kind);
                return skiptonl(t);
            }
            if (mg_state == MG_SIFNDEF) {
                mg_name = hash_string(t->rep);
                mg_state = MG_SMACRO;
            }
            cond_list->f.once = !(cond_list->f.ignore = mcr_redef(t->rep) ^ (kind == COND_KIFDEF));
            t = lxl_next();
            break;
        default:
            assert(!"invalid conditional kind -- should never reach here");
            break;
    }

    return skipsp(t);
}


/*
 *  accepts #elif
 */
static lex_t *delif(void)
{
    if (!cond_list)
        err_issuep(&lex_cpos, ERR_PP_NOMATCHIF, "#elif");
    else {
        if (!cond_list->prev)
            mg_state = MG_SINIT;
        if (cond_list->elsepos.y > 0)
            err_issuep(&lex_cpos, ERR_PP_ELIFAFTRELSE, &cond_list->elsepos);
        else if (cond_list->f.once)
            cond_list->f.ignore = 1;
        else if (cond_list->f.ignore != 2) {
            expr_t *c;
            lex_t *t = skipsp(lxl_next());

            if (t->id == LEX_NEWLINE)
                err_issuep(&lex_cpos, ERR_PP_NOIFEXPR, "#elif");
            else {
                c = expr_start(&t, "#elif");
                if (cond_list->f.once)
                    cond_list->f.ignore = 1;
                else
                    cond_list->f.once = !(cond_list->f.ignore = (c->u.u == 0));
            }

            return t;
        }
    }

    return lxl_next();
}


/*
 *  accepts #else
 */
static lex_t *delse(void)
{
    if (!cond_list)
        err_issuep(&lex_cpos, ERR_PP_NOMATCHIF, "#else");
    else {
        if (!cond_list->prev)
            mg_state = MG_SINIT;
        if (cond_list->elsepos.y > 0)
            err_issuep(&lex_cpos, ERR_PP_DUPELSE, &cond_list->elsepos);
        else {
            cond_list->elsepos = lex_cpos;
            if (cond_list->f.ignore != 2)
                cond_list->f.ignore = (cond_list->f.once)? 1: !cond_list->f.ignore;
        }
    }

    return skipsp(lxl_next());
}


/*
 *  accepts #endif
 */
static lex_t *dendif(void)
{
    if (!cond_list)
        err_issuep(&lex_cpos, ERR_PP_NOMATCHIF, "#endif");
    else {
        cond_pop();
        if (mg_state == MG_SMACRO && !cond_list) {
            mg_state = MG_SENDIF;
            state = SINIT;
        }
    }

    return skipsp(lxl_next());
}


/*
 *  accepts a digit sequence for line number
 */
static int digits(unsigned long *pn, const char *s, const lex_pos_t *ppos)
{
    int ovf = 0;
    unsigned long n = 0;

    assert(pn);
    assert(s);
    assert(ppos);

    while (isdigit(*s)) {
        if (n > (ULONG_MAX-(*s-'0')) / 10 || n > (TL_LINENO_STD-(*s-'0')) / 10)
            ovf = 1;
        n = 10*n + (*s++ - '0');
    }

    if (*s != '\0')
        return 0;

    if (ovf)
        err_issuep(PPOS(ppos), ERR_PP_LARGELINE);
    else if (n == 0)
        err_issuep(PPOS(ppos), ERR_PP_ZEROLINE);
    if (n == 0)
        n = 1;
    *pn = n;

    return 1;
}


/*
 *  recognizes a string literal including escape sequences
 */
static const char *recstr(const char *s)
{
    unsigned char *p, *r;

    assert(s);

    if (!strchr(s, '\\'))
        return s;

    p = r = ARENA_ALLOC(strg_perm, strlen(s)+1);    /* for filenames, thus strg_perm */
    while (*s) {
        if (*s == '\\') {
            s++;    /* skips \ */
            *p = lex_bs(&s, UCHAR_MAX, PPOS(&lex_cpos), "file name");
        } else
            *p = *s++;
        p++;
    }

    return (char *)r;
}


/*
 *  accepts #line
 */
static lex_t *dline(void)
{
    lex_t *t;
    int st = 0;    /* initial */
    unsigned long n;
    const char *fn = NULL;
    lex_pos_t epos = { 0, };

    while ((t=skipsp(lxl_next()))->id != LEX_NEWLINE) {
        assert(t->id != LEX_EOI);
        if (t->id == LEX_ID && !t->blue) {
            epos = lex_cpos;
            if (mcr_expand(t, &lex_cpos))
                continue;
        }
        if (st == 0) {    /* line number */
            if (t->id != LEX_PPNUM || !digits(&n, t->rep, &epos)) {
                err_issuep(PPOS(&epos), ERR_PP_ILLLINENO, t->rep);
                return t;
            }
            st = 1;
        } else if (st == 1) {    /* optional file name */
            if (t->id != LEX_SCON || *t->rep != '"') {
                err_issuep(PPOS(&epos), ERR_PP_ILLFNAME, t->rep);
                return t;
            }
            fn = recstr(t->rep);
            st = 2;
        } else {    /* extra tokens */
            err_issuep(PPOS(&epos), ERR_PP_EXTRATOKEN);
            t = skiptonl(t);
            break;
        }
    }

    if (st == 0)
        err_issuep(PPOS(&epos), ERR_PP_NOLINENO);
    else {
        in_cpos.my = n;
        if (fn)
            in_cpos.mf = hash_new(fn+1, strlen(fn+1)-1);
        if (!main_opt()->parsable) {
            assert(t->id == LEX_NEWLINE);
            in_cpos.c++;    /* for unique locus */
            in_cpos.y = n;
            ((lex_pos_t *)t->rep)->y = n - 1;    /* ty will be adjusted in outtok() */
            if (fn)
                ((lex_pos_t *)t->rep)->f = in_cpos.f = in_cpos.mf;
        }
    }

    return t;
}


/*
 *  accepts #error
 */
static lex_t *derror(void)
{
    lex_t *t;
    size_t len, n;
    lex_pos_t pos = lex_cpos;

    t = skipsp(lxl_next());
    if (t->id != LEX_NEWLINE) {
        strcpy(snbuf(len=2, 0), " ");
        do {
            assert(t->id != LEX_EOI);
            if (t->id == LEX_SPACE) {
                t = skipsp(lxl_next());
                if (t->id == LEX_NEWLINE)
                    break;
                strcpy(snbuf(len+1, 1)+len-1, " ");
                len++;
            }
            n = strlen(t->rep);
            strcpy(snbuf(len+n, 1)+len-1, t->rep);
            len += n;
        } while((t=lxl_next())->id != LEX_NEWLINE);
    } else
        *snbuf(len=1, 0) = '\0';

    err_issuep(&pos, (main_opt()->stricterr)? ERR_PP_ERROR2: ERR_PP_ERROR1, snbuf(len, 0));

    return t;
}


/*
 *  accepts #pragma
 */
static lex_t *dpragma(void)
{
    lex_t *t;
    int rec = 0;
    lex_pos_t pos = lex_cpos;

    t = skipsp(lxl_next());
    if (t->id == LEX_ID)
        t = prgm_start(t, &rec);
    if (!rec) {
        err_issuep(&pos, ERR_PP_UNKNOWNPRAGMA);
        t = skiptonl(t);
    }

    return t;
}


/*
 *  handles the "directive" state after the "normal" state
 */
static lex_t *direci(lex_t *t)
{
    int i = NELEM(dtab);

    assert(!cond_ignore());

    t = skipsp(t);
    if (t->id == LEX_ID) {
        if (snlen(t->rep, 8) < 8) {
            const char *s = hash_string(t->rep);
            for (i = 0; i < NELEM(dtab); i++)
                if (s == dtab[i].name)
                    break;
            switch(i) {
                case DINCLUDE:
                    t = dinclude();
                    break;
                case DDEFINE:
                    /* ddefine() moved into mcr.c for macros from -D */
                    t = mcr_define(0, lxl_next, &lex_cpos);
                    strg_tok = &strg_line;
                    break;
                case DUNDEF:
                    t = dundef();
                    break;
                case DIF:
                    t = dif(COND_KIF, 0);
                    break;
                case DIFDEF:
                    t = dif(COND_KIFDEF, 0);
                    break;
                case DIFNDEF:
                    t = dif(COND_KIFNDEF, 0);
                    break;
                case DELIF:
                    t = delif();
                    break;
                case DELSE:
                    t = delse();
                    break;
                case DENDIF:
                    t = dendif();
                    break;
                case DLINE:
                    t = dline();
                    break;
                case DERROR:
                    t = derror();
                    break;
                case DPRAGMA:
                    t = dpragma();
                    break;
                default:
                    err_issuep(&lex_cpos, ERR_PP_UNKNOWNDIR);
                    break;
            }
        } else
            err_issuep(&lex_cpos, ERR_PP_UNKNOWNDIR);
    } else
        i = DUNDEF;    /* to diagnose extra tokens for # */

    if (t->id != LEX_NEWLINE && warnxtra[i])
        err_issuep(&lex_cpos, ERR_PP_EXTRATOKEN);

    /* adds line sync and discards newline */
    outpos(((lex_pos_t *)skiptonl(t)->rep)->y+1, in_cpos.f, 0);
    lex_direc = 0;
    t = lxl_next(), reset();

    return t;
}


/*
 *  handles the "directive" state after the "ignore" state
 */
static lex_t *direce(lex_t *t)
{
    int i = NELEM(dtab), ign;

    assert(cond_ignore());
    ign = cond_list->f.ignore;

    t = skipsp(t);
    if (t->id == LEX_ID && snlen(t->rep, 8) < 8) {
        const char *s = hash_string(t->rep);
        for (i = 0; i < NELEM(dtab); i++)
            if (s == dtab[i].name)
                break;
        switch(i) {
            case DINCLUDE:
            case DDEFINE:
            case DUNDEF:
            case DLINE:
            case DERROR:
            case DPRAGMA:
            default:
                i = NELEM(dtab);
                break;
            case DIF:
                t = dif(COND_KIF, 1);
                break;
            case DIFDEF:
                t = dif(COND_KIFDEF, 1);
                break;
            case DIFNDEF:
                t = dif(COND_KIFNDEF, 1);
                break;
            case DELIF:
                t = delif();
                break;
            case DELSE:
                t = delse();
                break;
            case DENDIF:
                t = dendif();
                break;
        }
    }

    if (t->id != LEX_NEWLINE && warnxtra[i] && ign != 2)
        err_issuep(&lex_cpos, ERR_PP_EXTRATOKEN);

    /* adds line sync and discards newline */
    outpos(((lex_pos_t *)skiptonl(t)->rep)->y+1, in_cpos.f, 0);
    lex_direc = 0;
    t = lxl_next(), reset();

    return t;
}


/*
 *  handles the "normal" state
 */
static lex_t *norm(lex_t *t)
{
    assert(t);

    while (t->id != LEX_EOI) {
        if (t->id == LEX_ID && !t->blue)
            mcr_expand(t, &lex_cpos);
        else if (t->id == LEX_NEWLINE) {
            state = SAFTRNL;
            return lxl_next();
        }
        t = lxl_next();
    }

    return t;
}


/*
 *  handles the "ignore" state
 */
static lex_t *ign(lex_t *t)
{
    assert(t);

    while (t->id != LEX_EOI) {
        t = skipsp(t);
        if (t->id == LEX_SHARP) {
            state = SDIREC;
            lex_direc = 1;
            return lxl_next();
        } else if (t->id != LEX_NEWLINE)
            skiptonl(t);
        t = lxl_next();
    }

    return t;
}


/*
 *  sets directive and state based on cond_ignore()
 */
static void setdirecst(void)
{
    if (cond_ignore()) {
        state = SIGN;
        directive = direce;
    } else {
        switch(mg_state) {
            case MG_SINCLUDE:
                mg_state = MG_SINIT;
                state = SINIT;
                break;
            case MG_SENDIF:
                if (state == SINIT)
                    break;
                /* no break */
            default:
                state = SAFTRNL;
                break;
        }
        directive = direci;
    }
}


/*
 *  runs a FSM for preprocessing
 */
void (proc_start)(FILE *fp)
{
    int i;
    lex_t *t;

    assert(fp);

    for (i = 0; i < NELEM(dtab); i++)
        dtab[i].name = hash_string(dtab[i].name);

    outfile = fp;
    outpos(1, in_cpos.ff, -1);    /* always prints */
    ctx_init();
    setout(ctx_cur->cur);

    t = lxl_next();
    while (1) {
        while (t->id != LEX_EOI)
            switch(state) {
                case SINIT:
                case SAFTRNL:
                    t = aftrnl(t);
                    break;
                case SIDIREC:
                case SDIREC:
                    assert(lex_direc);
                    t = directive(t);
                    setdirecst();
                    break;
                case SNORM:
                    t = norm(t);
                    break;
                case SIGN:
                    t = ign(t);
                    break;
                default:
                    assert(!"invalid state -- should never reach here");
                    break;
            }

        if (mg_state == MG_SENDIF && state == SINIT)
            mg_once();
        cond_finalize();
        flush();
        if (inc_isffile())
            break;
        else {
            in_switch(NULL, "");    /* pop */
            setdirecst();
            assert(state == SAFTRNL || state == SINIT);
            assert(out->kind == LXL_KTOK && out->u.t.tok->id == LEX_EOI);
            outpos(in_cpos.y, in_cpos.f, 2);
            t = lxl_next();
            reset();
        }
    }
}

/* end of proc.c */
