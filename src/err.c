/*
 *  error handling
 */

#include <ctype.h>         /* is* */
#include <stdarg.h>        /* va_list, va_start, va_end */
#include <stddef.h>        /* NULL, size_t */
#include <stdio.h>         /* stderr, fprintf, vfprintf, sprintf, putc, fputs */
#include <string.h>        /* strlen */
#include <cbl/arena.h>     /* ARENA_ALLOC */
#include <cbl/assert.h>    /* assert */
#include <cbl/except.h>    /* except_t, EXCEPT_RAISE */

#include "common.h"
#include "in.h"
#include "main.h"
#ifdef SEA_CANARY
#include "../cpp/cond.h"
#include "../cpp/inc.h"
#include "../cpp/lex.h"
#else    /* !SEA_CANARY */
#include "lex.h"
#include "op.h"
#include "strg.h"
#include "simp.h"
#include "sym.h"
#include "tree.h"
#include "ty.h"
#endif    /* SEA_CANARY */
#include "err.h"

#ifndef SEA_CANARY
#define FIRST_EXPR LEX_ID                  /* FIRST(expr) */
#define FIRST_STMT LEX_IF                  /* FIRST(stmt) - FIRST(expr) */
#define FIRST_SPEC LEX_CHAR                /* FIRST_DECL - storage class */
#define FIRST_DECL LEX_CHAR, LEX_STATIC    /* FIRST(decl) - typedef id */
#endif    /* SEA_CANARY */

#ifdef HAVE_COLOR
#define ACRESET "\x1b[0m"
#ifndef ACDIAG
#define ACDIAG  "\x1b[39;1m"    /* default intensive */
#endif    /* !ACDIAG */
#ifndef ACLOCUS
#define ACLOCUS ACDIAG          /* same as ACDIAG */
#endif    /* !ACLOCUS */
#ifndef ACERR
#define ACERR   "\x1b[31;1m"    /* red intensive */
#endif    /* !ACERR */
#ifndef ACWARN
#define ACWARN  "\x1b[33;1m"    /* yellow intensive */
#endif    /* !ACWARN */
#ifndef ACNOTE
#define ACNOTE  "\x1b[32;1m"    /* green intensive */
#endif    /* !ACNOTE */
#ifndef ACQUOTE
#define ACQUOTE "\x1b[39m"      /* default */
#endif    /* !ACQUOTE */
#ifndef ACCARET
#define ACCARET "\x1b[32m"      /* green */
#endif    /* !ACCARET */
#endif    /* HAVE_COLOR */

#define dtype(f) ((f) & 0x03)    /* extracts diagnostic type */


/* error type/properties */
enum {
    /* diagnostic type; 0 indicates warning */
    E = 1,      /* error */
    N,          /* note */

    /* diagnostic properties */
    P = 1 << 2,    /* locus printed if set */
    O = 1 << 3,    /* issued once for file; works only with warnings */
    U = 1 << 4,    /* issued once for func; works only with warnings */
    T = 1 << 5,    /* issued once for tree; works only with warnings */
    F = 1 << 6,    /* fatal; not suppressed and compilation stops */
    A = 1 << 7,    /* warnings enabled when in C90 mode */
    B = 1 << 8,    /* warnings enabled when in C99 mode */
    C = 1 << 9,    /* warnings enabled when in C1X mode */
    W = 1 << 10    /* additional warnings issued when -W given or in standard mode */
};


int err_lim = 5;                                      /* # of allowed errors before stop */
const except_t err_except = { "too many errors" };    /* exception for too many errors */
/* err_cvbuf exposed just to manage storage */
#ifdef HAVE_ICONV
char *err_cvbuf;                                      /* encoding conversion buffer; see cvlen */
#endif    /* HAVE_ICONV */

#ifndef SEA_CANARY
/* predefined stop sets to handle syntax errors */
const char err_sset_field[] =    { FIRST_STMT, FIRST_SPEC, '}', 0 };
const char err_sset_strdef[] =   { FIRST_STMT, ',', 0 };
const char err_sset_enumdef[] =  { FIRST_STMT, 0 };
const char err_sset_decl[] =     { FIRST_DECL, ';', 0 };
const char err_sset_declf[] =    { FIRST_DECL, FIRST_EXPR, 0 };
const char err_sset_declb[] =    { FIRST_STMT, FIRST_DECL, FIRST_EXPR, 0 };
const char err_sset_expr[] =     { FIRST_STMT, FIRST_EXPR, '}', 0 };
const char err_sset_exprasgn[] = { FIRST_STMT, FIRST_EXPR, 0 };
const char err_sset_initf[] =    { FIRST_DECL, ';', 0 };
const char err_sset_initb[] =    { FIRST_STMT, FIRST_DECL, 0 };
#endif    /* !SEA_CANARY */


static int cnt;                   /* # of errors occurred */
#ifndef SEA_CANARY
static int inskip;                /* in skipping tokens */
#endif    /* !SEA_CANARY */
static int mute;                  /* > 0 if mute diagnostics requested */
#ifdef HAVE_ICONV
static size_t cvlen = TL_LINE;    /* size of err_cvbuf; see strg.c for initial value */
#endif    /* HAVE_ICONV */

/* diagnostic messages */
static const char *msg[] = {
#define xx(a, b, c, d) d,
#define yy(a, b, c, d) d,
#include "xerror.h"
};

/* error properties */
static int prop[]  = {
#define xx(a, b, c, d) b,
#define yy(a, b, c, d) b,
#include "xerror.h"
};

#ifndef SEA_CANARY
/* error flags for function */
static struct eff {
#define xx(a, b, c, d)
#define yy(a, b, c, d) unsigned a: 1;
#include "xerror.h"
} eff;

/* stack for diagnostic sites */
static struct plist {
    lex_pos_t pos;
    struct plist *next;
    struct plist *prev;
} phead, *pcur = &phead;
#endif    /* !SEA_CANARY */

/* turning-off flags for warnings */
static char nowarn[NELEM(msg)] = {
#define xx(a, b, c, d) c,
#define yy(a, b, c, d) c,
#include "xerror.h"
};

#ifdef SHOW_WARNCODE
static const char *wcode[NELEM(msg)];    /* driver options to control warnings */
#endif    /* SHOW_WARNCODE */


/*
 *  prepares to issue diagnostics
 */
void (err_init)(void)
{
#ifdef SHOW_WARNCODE
#define dd(a, b, c)
#define tt(a)
#define xx(a, b, c, d, e, f, g)
#ifdef SEA_CANARY
#define wpo(a, b, c) wcode[ERR_##b] = a;
#define wpx(a, b, c) wcode[ERR_##b] = a;
#define wco(a, b, c)
#define wcx(a, b, c)
#else    /* !SEA_CANARY */
#define wco(a, b, c) wcode[ERR_##b] = a;
#define wcx(a, b, c) wcode[ERR_##b] = a;
#define wpo(a, b, c)
#define wpx(a, b, c)
#endif    /* SEA_CANARY */
#include "../bcc/xopt.h"
#endif    /* SHOW_WARNCODE */
}


/*
 *  returns the number of errors occurred
 */
int (err_count)(void)
{
    return cnt;
}


#ifndef SEA_CANARY
/*
 *  terminates the token-skipping message
 */
void (err_skipend)(void)
{
    if (inskip) {
        if (mute == 0) {
            if (inskip > 1)
                fputs(" ...", stderr);
#ifdef HAVE_COLOR
            if (main_opt()->color)
                fputs(")\n"ACRESET, stderr);
            else
#endif    /* HAVE_COLOR */
                fputs(")\n", stderr);
        }
        inskip = 0;
    }
}


/*
 *  presumes that an expected token exists
 */
void (err_expect)(int tok)
{
    if (lex_tc == tok)
        lex_tc = lex_next();
    else
        err_issuex(ERR_PPREVE, ERR_PARSE_ERROR, tok, lex_tc);
}


/*
 *  prints an escape sequence for a non-printing character
 */
static void printesc(int c)
{
    const char *p;

    assert(!isprint((unsigned)c) || isspace((unsigned)c));

    switch(c) {
        case '\a':
            p = "\\a";
            break;
        case '\b':
            p = "\\b";
            break;
        case '\f':
            p = "\\f";
            break;
        case '\n':
            p = "\\n";
            break;
        case '\r':
            p = "\\r";
            break;
        case '\t':
            p = "\\t";
            break;
        case '\v':
            p = "\\v";
            break;
        case ' ':
            p = " ";
            break;
        default:
            if (main_opt()->hexcode)
                fprintf(stderr, ARBCHAR, (unsigned)c);
            else
                putc(c, stderr);
            return;
    }
    fputs(p, stderr);
}


/*
 *  prints the current token to stderr;
 *  ASSUMPTION: (HAVE_ICONV) UTF-8 used as internal pivot encoding
 */
static void printtok(void)
{
    assert(!main_opt()->parsable);
#ifdef HAVE_ICONV
    assert(err_cvbuf);
#endif    /* HAVE_ICONV */

    switch(lex_tc) {
        case LEX_ID:
            assert(lex_tok);
            fprintf(stderr, "'%s'", lex_tok);
            break;
        case LEX_ICON:
        case LEX_FCON:
            assert(in_cp);
            assert(lex_tok);
            assert((char *)in_cp > lex_tok);
            fprintf(stderr, "'%.*s'", (char *)in_cp-lex_tok, lex_tok);
            break;
        case LEX_CCON:
            {
                ty_t *t;
                int len;
                int c, d;
                const char *ell;
                const unsigned char *p, *q;

                c = '\'', d = '"';
                assert(lex_sym);
                t = lex_sym->type;
                goto scon;
        case LEX_SCON:
                c = '"', d = '\'';
                assert(lex_sym);
                assert(lex_sym->type);
                assert(lex_sym->type->op == TY_ARRAY);
                t = lex_sym->type->type;
            scon:
                assert(t);
                assert(ty_wchartype);    /* ensures types initialized */
                assert(lex_bp == &lex_buf);
                assert(lex_buf.s.p);
                len = 0;
                for (p = lex_buf.s.p; p < (unsigned char *)lex_tok; p++)
#ifdef HAVE_ICONV
                    if (main_ntoi) {
                        if (FIRSTUTF8(*p) && ++len > 10)
                            break;
                    } else
#endif    /* HAVE_ICONV */
                    if (++len > 10) {
                        len--;
                        break;
                    }
                ell = (p < (unsigned char *)lex_tok)? "...": "";
#ifdef HAVE_ICONV
                if (main_ntoi) {
                    ICONV_DECL((char *)lex_buf.s.p, p - lex_buf.s.p);
                    obuf = obufv = err_cvbuf;
                    olen = olenv = cvlen;
                    ICONV_DO(main_ntoi, 1, { (void)&pos; });
                    ICONV_INIT(main_ntoi);
                    q = (unsigned char *)(err_cvbuf = obuf), cvlen = olen;    /* for later reuse */
                    len = olen - olenv;
                } else
#endif    /* HAVE_ICONV */
                    q = lex_buf.s.p;
                fprintf(stderr, "%c%s%c", d, (t == ty_wchartype)? "L": "", c);
                for (p = q; p < q + len; p++) {
                    if (isprint(*p) && !isspace(*p))
                            putc(*p, stderr);
                        else
                            printesc(*p);
                }
                fprintf(stderr, "%s%c%c", ell, c, d);
            }
            break;
        default:
            fprintf(stderr, "'%s'", lex_name[lex_tc]);
            break;
    }
}


/*
 *  skips tokens until an expected one appears
 */
void (err_skipto)(int tok, const char set[])
{
    int n;
    const char *s;

    assert(set);

    for (n = 0; lex_tc != LEX_EOI && lex_tc != tok; lex_tc = lex_next()) {
        for (s = set; *s; s++)
            if (lex_kind[lex_tc] == *s || lex_tc == *s)
                goto match;
        if (main_opt()->parsable)
            n++;
        else {
            if (!inskip) {
                inskip = 1;
                n = 0;
                if (mute == 0) {
#if HAVE_COLOR
                    if (main_opt()->color)
                        fputs(ACRESET"  "ACQUOTE"(skipping", stderr);
                    else
#endif    /* HAVE_COLOR */
                        fputs("  (skipping", stderr);
                }
            }
            if (++n <= 5) {
                if (mute == 0) {
                    putc(' ', stderr);
                    printtok();
                }
            } else
                inskip = 2;
        }
    }
    match:
        if (main_opt()->parsable) {
            if (n > 0)
                err_issuex(ERR_PPREVS, ERR_PARSE_SKIPTOK, (long)n, "token");
        } else {
            if (inskip && n > 5 && mute == 0) {
                fputs(" ... up to ", stderr);
                printtok();
                inskip = 1;
            }
            err_skipend();
        }
}


/*
 *  tests if the next token is tok
 */
void (err_test)(int tok, const char set[])
{
    if (lex_tc != tok) {
        err_expect(tok);
        err_skipto(tok, set);
    }
    if (lex_tc == tok)
        lex_tc = lex_next();
}


/*
 *  turns off all diagnostics except fatal ones in a nestable way
 */
void (err_mute)(void)
{
    mute++;
}


/*
 *  turns on all diagnostics in a nestable way
 */
void (err_unmute)(void)
{
    assert(mute > 0);
    mute--;
}
#endif    /* !SEA_CANARY */


/*
 *  enables or disables a warning;
 *  no effect on errors and messages skipping tokens
 */
void (err_nowarn)(int code, int flag)
{
    if (code >= 0 && code < NELEM(nowarn))
        nowarn[code] = !!flag;
}


#ifdef HAVE_ICONV
/*
 *  adjusts a locus;
 *  ASSUMPTION: (HAVE_ICONV) UTF-8 used as internal pivot encoding
 */
static unsigned long adjustx(const unsigned char **buf, unsigned long x)
{
    int i = 0;
    size_t tlen = strlen((const char *)*buf) + 1;
    ICONV_DECL((char *)*buf, 0);

    assert(buf);
    assert(*buf);
    assert(err_cvbuf);

    while (!FIRSTUTF8((*buf)[x-1]) && x > 1 && i++ < 5)
        x--;
    ilenv = x - 1;
    obufv = obuf = err_cvbuf;
    olenv = olen = cvlen;
    for (i = 0; i < 2; i++) {
        ICONV_DO(main_ntoi, !i, { (void)&pos; });
        if (i == 0) {
            ilenv += tlen - (x-1);
            x = olen - olenv + 1;
        }
    }
    err_cvbuf = obuf, cvlen = olen;    /* for later reuse */
    *buf = (unsigned char *)obuf;

    return x;
}
#endif    /* HAVE_ICONV */


/*
 *  prints a source line considering tabstops;
 *  ASSUMPTION: isprint() is true for only chars with one-byte position
 */
static void putline(const unsigned char *buf, unsigned long x)
{
    int tab;
    unsigned long col;
    const unsigned char *p;

    assert(buf);

#ifdef HAVE_COLOR
    if (main_opt()->color)
        fputs(ACRESET"  "ACQUOTE, stderr);
    else
#endif    /* HAVE_COLOR */
        fputs("  ", stderr);

    tab = main_opt()->tabstop;
    assert(tab > 1);

    col = 0;
    for (p = buf; *p != '\0'; p++) {
        if (*p == '\t') {
            int i = tab - (col % tab);
            if (x-1 > col)
                x += i-1;
            col += i;
            while (i-- > 0)
                putc(' ', stderr);
        } else {
            if (*p == '\v' || *p == '\f' || *p == '\r')
                putc(' ', stderr);
            else if (main_opt()->hexcode) {
                if (isprint(*p) || isspace(*p))
                    putc(*p, stderr);
                else {
                    int k = fprintf(stderr, ARBCHAR, (unsigned)*p) - 1;
                    if (x-1 > col)
                        x += k;
                    col += k;
                }
            } else
                putc(*p, stderr);
            col++;
        }
    }

#ifdef HAVE_COLOR
    if (main_opt()->color)
        fputs(ACRESET, stderr);
#endif    /* HAVE_COLOR */
    while (x-- > 1)    /* x-1 spaces */
        putc(' ', stderr);
#ifdef HAVE_COLOR
    if (main_opt()->color)
        fputs("  "ACCARET"^\n"ACRESET, stderr);
    else
#endif    /* HAVE_COLOR */
        fputs("  ^\n", stderr);
}


#ifndef SEA_CANARY
/*
 *  sets an error flag of a function
 */
static int seteff(int code)
{
    switch(code) {
#define xx(a, b, c, d)
#define yy(a, b, c, d) case ERR_##a: if (eff.a) return 1; else eff.a = 1; break;
#include "xerror.h"
        default:
            assert(!"invalid error code -- should never reach here");
            break;
    }

    return 0;
}


/*
 *  clears error flags of a function
 */
void (err_cleareff)(void)
{
    static struct eff clear = { 0, };
    eff = clear;
}


/*
 *  sets an error flag of a tree
 */
static int seteft(tree_t *tp, int code)
{
    assert(tp);

    if (tp->op == OP_RIGHT)
        while (tp->kid[1] && tp->kid[1]->op == OP_RIGHT && ty_same(tp->kid[1]->type, tp->type))
            tp = tp->kid[1];

    switch(code) {
#define xx(a, b, c, d)
#define yy(a, b, c, d) case ERR_##a: if (tp->f.a) return 1; else tp->f.a = 1; break;
#include "xerror.h"
        default:
            assert(!"invalid error code -- should never reach here");
            break;
    }

    return 0;
}


/*
 *  returns a string for an identifier; " `id'" or noid
 */
static const char *symstr(const sym_t *p, const char *noid)
{
    static char buf[64+1];

    tree_t *t;
    size_t n;
    char *pbuf = buf;

    assert(noid);

    if (p && (t = simp_basetree(p, NULL)) != NULL)
        p = t->u.sym;

    if (!p || !p->name || GENNAME(p->name) || *p->name == '#' || p->f.computed)
        return noid;
    if ((n = 2+strlen(p->name)+1+1) > sizeof(buf))
        pbuf = ARENA_ALLOC(strg_stmt, n);
    sprintf(pbuf, " `%s'", p->name);

    return pbuf;
}
#endif    /* !SEA_CANARY */


/*
 *  returns a string for an ordinal number
 */
static const char *ordinal(unsigned n)
{
    static char buf[BUFN + 2 + 1];

    unsigned m;

    m = (n > 20)? n % 10: n;
    sprintf(buf, "%u%s", n, (m == 1)? "st": (m == 2)? "nd": (m == 3)? "rd": "th");

    return buf;
}


/*
 *  prints a diagnostic message with custom format characters
 */
static void fmt(const char *s, va_list ap)
{
    int n;
    char c;
#ifndef SEA_CANARY
    const sym_t *p;
    const ty_t *ty;
#endif    /* !SEA_CANARY */

    assert(s);

    while ((c = *s++) != '\0') {
        if (c == '%')
            switch(c = *s++) {
#ifdef SEA_CANARY
                case 'C':    /* conditional kind */
                    fputs(cond_name(va_arg(ap, int)), stderr);
                    break;
#else    /* !SEA_CANARY */
                case 'C':    /* type category */
                    fputs(ty_outcat(va_arg(ap, ty_t *)), stderr);
                    break;
                case 'D':    /* declaration - ty *, char *, int * */
                    {
                        char *id;
                        int a, *pa;

                        ty = va_arg(ap, ty_t *);
                        id = va_arg(ap, char *);
                        pa = va_arg(ap, int *);
                        fprintf(stderr, "`%s'", ty_outdecl(ty, id, pa, 0));
                        if (ty_hastypedef(ty))
                            fprintf(stderr, " (aka `%s')", ty_outdecl(ty, id, &a, 1));
                    }
                    break;
                case 'f':    /* function name */
                    fputs(tree_fname(va_arg(ap, tree_t *)), stderr);
                    break;
                case 'i':    /* id - char *, char * */
                    p = err_idsym(va_arg(ap, char *));
                    fputs(symstr(p, va_arg(ap, char *)), stderr);
                    break;
                case 'I':    /* id - sym_t *, char * */
                    p = va_arg(ap, sym_t *);
                    fputs(symstr(p, va_arg(ap, char *)), stderr);
                    break;
                case 't':    /* token name */
                    fputs(lex_name[va_arg(ap, int)], stderr);
                    break;
                case 'y':    /* type with typedef preserved */
                    ty = va_arg(ap, ty_t *);
                    fprintf(stderr, "`%s'", ty_outtype(ty, 0));
                    if (ty_hastypedef(ty))
                        fprintf(stderr, " (aka `%s')", ty_outtype(ty, 1));
                    break;
#endif    /* SEA_CANARY */
                case 'c':    /* char */
                    putc(va_arg(ap, int), stderr);
                    break;
                case 'd':    /* long */
                    fprintf(stderr, "%ld", va_arg(ap, long));
                    break;
                case 'o':    /* ordinal */
                    fputs(ordinal(va_arg(ap, unsigned)), stderr);
                    break;
                case 'p':    /* locus */
                    fputs(lex_outpos(va_arg(ap, lex_pos_t *)), stderr);
                    break;
                case 'P':    /* plural - int, char * */
                    n = va_arg(ap, int);
                    fprintf(stderr, "%d %s%s", n, va_arg(ap, char *), (n > 1)? "s": "");
                    break;
                case 's':    /* char * */
                    fputs(va_arg(ap, char *), stderr);
                    break;
                case 'u':    /* unsigned long */
                    fprintf(stderr, "%lu", va_arg(ap, unsigned long));
                    break;
                default:
                    putc('%', stderr);
                    putc(c, stderr);
                    break;
            }
        else
            putc(c, stderr);
    }
}


/*
 *  escapes colons in a file name for parsable diagnostics
 */
static void esccolon(const char *s)
{
    for (; *s; s++) {
        if (*s == ':' || *s == '\\')
            putc('\\', stderr);
        putc(*s, stderr);
    }
}


/*
 *  issues a diagnostic message
 */
static void issue(const lex_pos_t *ppos, int code, va_list ap)
{
    int t;
    unsigned long y, x;
    const unsigned char *p;

    assert(ppos);
    assert(code >= 0 && code < NELEM(msg));
    assert(in_cpos.ff);
    assert(ppos->f);
#ifndef SEA_CANARY
    assert(in_incp);
#endif    /* !SEA_CANARY */
    assert(msg[code]);

    t = dtype(prop[code]);
    y = (prop[code] & P)? ppos->y: 0;
    x = (y == 0)? 0: ppos->x;

    if (!(prop[code] & F) && ((t != E && nowarn[code]) || mute > 0))    /* message suppressed */
        return;
    if ((prop[code] & W) && !main_opt()->addwarn && !main_opt()->std)    /* additional warn */
        return;
    if ((prop[code] & (A|B|C)) &&
        !(((prop[code] & A) && main_opt()->std == 1) ||      /* C90 warning */
          ((prop[code] & B) && main_opt()->std == 2) ||      /* C99 warning */
          ((prop[code] & C) && main_opt()->std == 3)))       /* C1X warning */
        return;
    if (prop[code] & O)
        nowarn[code] = 1;
#ifndef SEA_CANARY
    else if (prop[code] & U) {
        if (seteff(code))
            return;
    } else if (prop[code] & T) {
        tree_t *tp = va_arg(ap, tree_t *);
        if (seteft(tp, code))
            return;
    }
#endif    /* !SEA_CANARY */

    /* ff, fy, f */
    if (main_opt()->parsable) {
#ifdef HAVE_COLOR
        if (main_opt()->color)
            fputs(ACLOCUS, stderr);
#endif    /* HAVE_COLOR */
        esccolon(in_cpos.ff);
        putc(':', stderr);
        if (y > 0)
            fprintf(stderr, "%lu", ppos->fy);
        putc(':', stderr);
        if (!in_cpos.n)
            esccolon(ppos->f);
        putc(':', stderr);
    } else {
#ifdef SEA_CANARY
#define ADJ 1
        inc_t *p, **pi = inc_list;
#else    /* !SEA_CANARY */
#define ADJ 0
        in_inc_t *p, **pi = in_incp;
#endif    /* SEA_CANARY */

#ifdef SEA_CANARY
        assert(inc_list);
        assert(*inc_list);
#else    /* !SEA_CANARY */
        err_skipend();
#endif    /* SEA_CANARY */
#ifdef HAVE_COLOR
        if (main_opt()->color)
            fputs(ACLOCUS, stderr);
#endif    /* HAVE_COLOR */
        p = *pi;
        if (p->f && !p->printed) {
            assert(p->y - ADJ > 0);
            p->printed = 1;
            fprintf(stderr, "In file included from %s:%lu", p->f, p->y-ADJ);
            while ((p = *++pi)->f) {
                assert(p->y-ADJ > 0);
                fprintf(stderr, ",\n                 from %s:%lu", p->f, p->y-ADJ);
            }
            fputs(":\n", stderr);
        }
        fprintf(stderr, "%s:", ppos->f);
#undef ADJ
    }

    /* y, x */
    if (y)
        fprintf(stderr, "%lu", y);
    if (main_opt()->parsable || y)
        putc(':', stderr);
#ifdef HAVE_ICONV
    if (x) {
        if (main_ntoi && (p = in_getline(ppos->c, ppos->f, y)) != NULL)
            x = adjustx(&p, x);
        fprintf(stderr, "%lu", x);
    }
#else    /* !HAVE_ICONV */
    if (x)
        fprintf(stderr, "%lu", x);
#endif    /* HAVE_ICONV */
    if (main_opt()->parsable || x)
        putc(':', stderr);

    {    /* diagnostic */
        static const char *label[] = { "warning", "ERROR", "note" };
#ifdef HAVE_COLOR
        static const char *color[] = { ACWARN,    ACERR,    ACNOTE };
#endif    /* HAVE_COLOR */

        if (main_opt()->warnerr && t != N)
            t = E;
#ifdef HAVE_COLOR
        if (main_opt()->color)
            fprintf(stderr, ACRESET" %s%s"ACRESET" - "ACDIAG, color[t], label[t]);
        else
#endif    /* HAVE_COLOR */
            fprintf(stderr, " %s - ", label[t]);
        fmt(msg[code], ap);
#ifdef SHOW_WARNCODE
        if (main_opt()->warncode && t != E && wcode[code])
            fprintf(stderr, " [-W%s]", wcode[code]);
#endif    /* SHOW_WARNCODE */
        putc('\n', stderr);
    }

    /* source line */
    if (!main_opt()->parsable && main_opt()->showsrc && x
#ifdef HAVE_ICONV
        && ((main_ntoi && p) || (!main_ntoi && (p = in_getline(ppos->c, ppos->f, y)) != NULL))
#else    /* !HAVE_ICONV */
        && (p = in_getline(ppos->c, ppos->f, y)) != NULL
#endif    /* HAVE_ICONV */
       )
        putline(p, x);
#ifdef HAVE_COLOR
    else if (main_opt()->color)
        fputs(ACRESET, stderr);
#endif    /* HAVE_COLOR */

    if (prop[code] & F) {
        cnt = -1;
        EXCEPT_RAISE(err_except);
    }

    if (prop[code] & O)
        err_issue(ERR_XTRA_ONCEFILE);

    if (t == E && ++cnt >= err_lim && err_lim > 0) {
        assert(prop[ERR_XTRA_ERRLIMIT] & F);
        err_issue(ERR_XTRA_ERRLIMIT);
    }
}


#ifndef SEA_CANARY
/*
 *  enter a site with setting a locus for diagnostics;
 *  every function that depends on a diagnostic site have a name that ends with "_s";
 *  NULL given to turn off diagnostics for some cases
 */
void (err_entersite)(const lex_pos_t *ppos)
{
    static lex_pos_t null;

    if (!pcur->next) {
        pcur->next = ARENA_ALLOC(strg_perm, sizeof(*pcur->next));
        pcur->next->prev = pcur;
        pcur->next->next = NULL;
    }
    pcur = pcur->next;
    pcur->pos = (ppos)? *ppos: null;
}


/*
 *  returns a locus from the current diagnostic site
 */
const lex_pos_t *(err_getppos)(void)
{
    assert(pcur->pos.y > 0);
    return &pcur->pos;
}


/*
 *  exits a site for diagnostics
 */
void (err_exitsite)(void)
{
    assert(pcur->prev);
    pcur = pcur->prev;
}


/*
 *  issues a diagnostic message using a diagnostic site (va_list version)
 */
static void issue_s(int code, va_list ap)
{
    assert(code >= 0 && code < NELEM(prop));

    if (pcur->pos.y == 0)
        return;

    issue(&pcur->pos, code, ap);
}


/*
 *  issues a diagnostic message using a diagnostic site
 */
void (err_issue_s)(int code, ...)
{
    va_list ap;

    va_start(ap, code);
    issue_s(code, ap);
    va_end(ap);
}


/*
 *  issues a diagnostic message using lex_[c|p]pos
 */
void (err_issuex)(int cp, int code, ...)
{
    static lex_pos_t **arr[] = { &lex_ppos, NULL, &lex_cpos };

    va_list ap;

    assert(cp >= 0 || cp < NELEM(arr));
    assert(code >= 0 && code < NELEM(prop));

    va_start(ap, code);
    issue((cp == ERR_PPREVE)? lex_epos(): *arr[cp], code, ap);
    va_end(ap);
}
#endif    /* !SEA_CANARY */


/*
 *  issues a diagnostic message using lex_pos_t;
 *  nothing issued if a null pointer given to ppos
 */
void (err_issuep)(const lex_pos_t *ppos, int code, ...)
{
    va_list ap;

    assert(code >= 0 && code < NELEM(prop));

    if (!ppos)
        return;
    va_start(ap, code);
    issue(ppos, code, ap);
    va_end(ap);
}


/*
 *  issues a diagnostic message using in_cp
 */
void (err_issue)(int code, ...)
{
    va_list ap;
    lex_pos_t pos;

    assert(code >= 0 && code < NELEM(prop));

    pos.c = in_cpos.c;
    pos.fy = in_cpos.fy;
    pos.f = in_cpos.f;
    pos.y = in_cpos.y;
    pos.x = (in_line && in_cp && in_cp >= in_line)? in_cp-in_line+in_outlen+1: 0;
#ifndef SEA_CANARY
    pos.n = 0;
#endif    /* !SEA_CANARY */

    va_start(ap, code);
    issue(&pos, code, ap);
    va_end(ap);
}


#ifndef SEA_CANARY
/*
 *  issues an expression type error using a diagnostic site
 */
void (err_experr_s)(int experr, int code, ...)
{
    va_list ap;

    if (experr)
        return;

    va_start(ap, code);
    issue_s(code, ap);
    va_end(ap);
}


/*
 *  issues an expression type error using lex_pos_t;
 *  nothing issued if a null pointer given to ppos
 */
void (err_experrp)(int experr, const lex_pos_t *ppos, int code, ...)
{
    va_list ap;

    assert(ppos);
    assert(code >= 0 && code < NELEM(prop));

    if (!ppos || experr)
        return;
    va_start(ap, code);
    issue(ppos, code, ap);
    va_end(ap);
}


/*
 *  boxes an identifier to make a symbol
 */
const sym_t *(err_idsym)(const char *id)
{
    static sym_t sym;

    sym.name = id;
    return &sym;
}
#endif    /* !SEA_CANARY */

/* end of err.c */
