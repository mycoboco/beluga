/*
 *  error handling
 */

#include <stdarg.h>        /* va_list, va_start, va_end */
#include <stddef.h>        /* NULL */
#include <stdio.h>         /* stderr, fprintf, sprintf, putc, fputs */
#include <string.h>        /* strlen */
#include <cbl/arena.h>     /* ARENA_ALLOC */
#include <cbl/assert.h>    /* assert */
#include <cbl/except.h>    /* except_t, EXCEPT_RAISE */

#include "clx.h"
#include "common.h"
#include "cond.h"
#include "in.h"
#include "lmap.h"
#include "main.h"
#include "simp.h"
#include "strg.h"
#include "sym.h"
#include "tree.h"
#include "ty.h"
#include "err.h"

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
    E = 1,    /* error */
    N,        /* note */

    /* diagnostic properties */
    P = 1 << 2,    /* locus printed if set */

    O = 1 << 3,    /* issued once for file; works only with warnings */
    U = 1 << 4,    /* issued once for func; works only with warnings */
    X = 1 << 5,    /* errors to stop tree generation */

    F = 1 << 6,    /* fatal; never suppressed and compilation stops */
    A = 1 << 7,    /* warnings enabled when in C90 mode */
    B = 1 << 8,    /* warnings enabled when in C99 mode */
    C = 1 << 9,    /* warnings enabled when in C1X mode */
    W = 1 << 10    /* additional warnings issued when -W given or in standard mode */
};


int err_mute;                                         /* true if diagnostics suppressed */
int err_lim = 5;                                      /* # of allowed errors before stop */
const except_t err_except = { "too many errors" };    /* exception for too many errors */


static int cnt;    /* # of errors occurred */

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

/* error flags for function */
static struct eff {
#define xx(a, b, c, d)
#define yy(a, b, c, d) unsigned a: 1;
#include "xerror.h"
    unsigned x: 1;
} eff;

/* turning-off flags for warnings */
static char nowarn[NELEM(msg)] = {
#define xx(a, b, c, d) c,
#define yy(a, b, c, d) c,
#include "xerror.h"
};

/* locus slots for diagnostics */
static struct epos_t {
    const char *rpf;        /* resolved physical file name */
    const char *pf;         /* physical file name */
    sz_t py;                /* physical line # */
    const char *f;          /* nominal file name */
    sz_t y;                 /* nominal line # */
    sz_t wx;                /* x counted by wcwidth() */
    int dy;                 /* extra lines */
    sz_t dx;                /* x at which range ends; counted by wcwidth() */
    struct epos_t *next;    /* next locus */
} *eposs[3];

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
#define wpo(a, b, c) wcode[ERR_##b] = a;
#define wpx(a, b, c) wcode[ERR_##b] = a;
#define wco(a, b, c) wcode[ERR_##b] = a;
#define wcx(a, b, c) wcode[ERR_##b] = a;
#include "../bcc/xopt.h"
#endif    /* SHOW_WANRCODE */
}


/*
 *  returns the number of errors occurred
 */
int (err_count)(void)
{
    return cnt;
}


/*
 *  enables or disables a warning;
 *  no effect on errors
 */
void (err_nowarn)(int code, int off)
{
    if (code >= 0 && code < NELEM(nowarn))
        nowarn[code] = off;
}


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
 *  checks if stopping tree generation has been occurred
 */
int (err_experr)(void)
{
    return eff.x;
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
 *  composes a locus for diagnostics;
 *  intended to be consumed immediately
 */
static struct epos_t *epos(const lmap_t *h, sz_t py, sz_t wx, int n, struct epos_t *q)
{
    static struct epos_t ep;

    struct epos_t *p = (q)? ARENA_ALLOC(strg_line, sizeof(*p)): &ep;

    if (h) {    /* token locus */
        switch(h->type) {
            case LMAP_AFTER:
                h = h->from;
                while (h->type == LMAP_MACRO)
                    h = h->u.m;
                assert(h->type == LMAP_NORMAL);
                py = h->u.n.py + h->u.n.dy;
                p->wx = h->u.n.dx;
                p->dy = 0;
                p->dx = p->wx + 1;
                break;
            case LMAP_PIN:
                h = h->from;
                while (h->type == LMAP_MACRO)
                    h = h->u.m;
                assert(h->type == LMAP_NORMAL);
                py = h->u.n.py;
                p->wx = h->u.n.wx;
                p->dy = 0;
                p->dx = p->wx + 1;
                break;
            default:
                while (h->type == LMAP_MACRO)
                    h = h->u.m;
                assert(h->type == LMAP_NORMAL);
                py = h->u.n.py;
                p->wx = h->u.n.wx;
                p->dy = h->u.n.dy;
                p->dx = h->u.n.dx;
                break;
        }
    } else {    /* lmap_from + locus */
        h = lmap_from;
        p->wx = wx;
        p->dy = 0;
        p->dx = wx + n;
    }
    assert(py > 0);
    assert(p->dy > 0 || p->dx > p->wx);

    h = lmap_nfrom(h);
    p->f = (h->type == LMAP_LINE)? h->u.l.f: NULL;
    p->y = py + h->u.i.yoff;    /* cis */

    h = lmap_pfrom(h);
    p->rpf = h->u.i.rf;
    p->pf = h->u.i.f;
    p->py = py;
    if (!p->f)
        p->f = p->pf;
    p->next = q;

    return p;
}


#define SWAP(p, q) (t = (p), (p) = (q), (q) = t)

/*
 *  prepares diagnostic loci
 */
static int prep(struct epos_t *ep, const char *s)
{
    int n;
    struct epos_t *t;
    sz_t end = (sz_t)-1;

    assert(ep);
    assert(s);

    for (n=0, t=ep; n < NELEM(eposs) && t; t = t->next) {
         if (ep->rpf != t->rpf)
             continue;
         if (t->py == ep->py) {
            if (t->dy > 0) {
                if (end == (sz_t)-1) {
                    const char *q = s + strlen(s);
                    end = in_getwx(1, s, q, NULL);
                }
                t->dx = end;
            }
            if (t == ep || ep->wx < t->wx || t->dx <= ep->wx)
                eposs[n++] = t;
        } else if (t->py == ep->py+ep->dy) {
            assert(t != ep);
            t->wx = 1;
            if (t->dx < ep->wx)
                eposs[n++] = t;
        }
    }

    if (n > 1) {
        if (eposs[0]->wx > eposs[1]->wx)
            SWAP(eposs[0], eposs[1]);
        if (n > 2) {
            if (eposs[0]->wx > eposs[2]->wx)
                SWAP(eposs[0], eposs[2]);
            if (eposs[1]->wx > eposs[2]->wx)
                SWAP(eposs[1], eposs[2]);
        }
    }

    assert(n > 0);
    return n;
}

#undef SWAP


/*
 *  prints a source line
 */
static void putline(struct epos_t *ep)
{
    int n;
    const char *p;

    assert(ep);

    p = lmap_flget(ep->rpf, ep->py);
    if (!p)
        return;

    n = prep(ep, p);

#ifdef HAVE_COLOR
    if (main_opt()->color)
        fputs(ACRESET"  "ACQUOTE, stderr);
    else
#endif    /* HAVE_COLOR */
        fputs("  ", stderr);
    while (*p != '\0') {
        putc((ISCH_SP(*p))? ' ': *p, stderr);
        p++;
    }

#ifdef HAVE_COLOR
    if (main_opt()->color)
        fputs(ACRESET"\n  ", stderr);
    else
#endif    /* HAVE_COLOR */
        fputs("\n  ", stderr);
    {
        int i;
        sz_t c;
        for (i=0, c=1; i < n; i++) {
            while (1) {
                if (c < eposs[i]->wx)
                    putc(' ', stderr);
                else if (c == eposs[i]->wx) {
#ifdef HAVE_COLOR
                    if (main_opt()->color)
                        fputs((eposs[i] == ep)? ACCARET"^": ACCARET"~", stderr);
                    else
#endif    /* HAVE_COLOR */
                        fputs((eposs[i] == ep)? "^": "~", stderr);
                } else if (c > eposs[i]->wx && c < eposs[i]->dx)
                    putc('~', stderr);
                else if (c >= eposs[i]->dx) {
#ifdef HAVE_COLOR
                    if (main_opt()->color)
                        fputs(ACRESET, stderr);
#endif    /* HAVE_COLOR */
                    break;
                }
                c++;
            }
        }
    }
    putc('\n', stderr);
}


/*
 *  returns a string for an identifier; " `id'" or noid
 */
static const char *symstr(const sym_t *p, const char *noid)
{
    static char buf[64+1];

    tree_t *t;
    sz_t n;
    char *pbuf = buf;

    assert(noid);

    if (p && (t = simp_basetree(p, NULL)) != NULL)
        p = t->u.sym;

    if (!p || !p->name || GENNAME(p->name) || *p->name == '#' || p->f.computed)
        return noid;
    if ((n = 2+strlen(p->name)+1+1) > sizeof(buf))
        pbuf = ARENA_ALLOC(strg_line, n);
    sprintf(pbuf, " `%s'", p->name);

    return pbuf;
}


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
    const sym_t *p;
    const ty_t *ty;

    assert(s);

    while ((c = *s++) != '\0') {
        if (c == '%')
            switch(c = *s++) {
                case 'C':    /* type category */
                    fputs(ty_outcat(va_arg(ap, ty_t *)), stderr);
                    break;
                case 'c':    /* char */
                    putc(va_arg(ap, int), stderr);
                    break;
                case 'D':    /* declaration - ty *, char *, int * */
                    {
                        char *id;
                        int a, *pa;

                        ty = va_arg(ap, ty_t *);
                        id = va_arg(ap, char *);
                        pa = va_arg(ap, int *);
                        fprintf(stderr, "`%s'", ty_outdecl(ty, id, pa, 0));
                        if (ty_hastypedef(ty) && !TY_ISUNKNOWN(ty))
                            fprintf(stderr, " (aka `%s')", ty_outdecl(ty, id, &a, 1));
                    }
                    break;
                case 'd':    /* long */
                    fprintf(stderr, "%ld", va_arg(ap, long));
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
                case 'k':    /* conditional kind */
                    fputs(cond_name(va_arg(ap, int)), stderr);
                    break;
                case 'o':    /* ordinal */
                    fputs(ordinal(va_arg(ap, unsigned)), stderr);
                    break;
                case 'P':    /* plural - int, char * */
                    n = va_arg(ap, int);
                    fprintf(stderr, "%d %s%s", n, va_arg(ap, char *), (n > 1)? "s": "");
                    break;
                case 's':    /* char * */
                    fputs(va_arg(ap, char *), stderr);
                    break;
                case 't':    /* token name */
                    fputs(clx_name[va_arg(ap, int)], stderr);
                    break;
                case 'u':    /* unsigned long */
                    fprintf(stderr, "%lu", va_arg(ap, unsigned long));
                    break;
                case 'y':    /* type with typedef preserved */
                    ty = va_arg(ap, ty_t *);
                    fprintf(stderr, "`%s'", ty_outtype(ty, 0));
                    if (main_opt()->unwind && ty_hastypedef(ty) && !TY_ISUNKNOWN(ty))
                        fprintf(stderr, " (aka `%s')", ty_outtype(ty, 1));
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
 *  issues a diagnostic message
 */
static int issue(struct epos_t *ep, const lmap_t *from, int code, va_list ap)
{
    int t;
    sz_t iy, y, x;
    const char *rpf;
    const lmap_t *pos;

    assert(ep);
    assert(ep->f);
    assert(from);
    assert(code >= 0 && code < NELEM(msg));
    assert(msg[code]);

    t = dtype(prop[code]);
    y = (prop[code] & P)? ep->y: 0;
    x = (y == 0)? 0: ep->wx;

    if (from->type >= LMAP_AFTER)
        from = from->from;
    pos = lmap_pfrom((from->type == LMAP_MACRO)? from->u.m: from);
    if (!(prop[code] & F) && ((t != E && (nowarn[code] ||
                                          (t != N && pos->type == LMAP_INC && pos->u.i.system))) ||
                              err_mute))
        return 0;
    if ((prop[code] & W) && !main_opt()->addwarn && !main_opt()->std)    /* additional warning */
        return 0;
    if ((prop[code] & (A|B|C)) &&
        !(((prop[code] & A) && main_opt()->std == 1) ||    /* C90 warning */
          ((prop[code] & B) && main_opt()->std == 2) ||    /* C99 warning */
          ((prop[code] & C) && main_opt()->std == 3)))     /* C1X warning */
        return 0;
    if (prop[code] & O)
        nowarn[code] = 1;
    else if (prop[code] & U) {
        if (seteff(code))
            return 0;
    } else if (prop[code] & X)
        eff.x = 1;

    /* #include chain */
    if (pos->type == LMAP_INC && !pos->u.i.printed) {
#ifdef HAVE_COLOR
        if (main_opt()->color)
            fputs(ACLOCUS, stderr);
#endif    /* HAVE_COLOR */
        ((lmap_t *)pos)->u.i.printed = 1;
        assert(pos->from->type == LMAP_NORMAL);
        iy = pos->from->u.n.py;
        pos = lmap_nfrom(pos->from);
        rpf = pos->u.i.f;      /* cis */
        iy += pos->u.i.yoff;    /* cis */
        fprintf(stderr, "In file included from %s:%"FMTSZ"u", rpf, iy);
        while (pos->type == LMAP_INC) {
            assert(pos->from->type == LMAP_NORMAL);
            iy = pos->from->u.n.py;
            pos = lmap_nfrom(pos->from);
            rpf = pos->u.i.f;       /* cis */
            iy += pos->u.i.yoff;    /* cis */
            fprintf(stderr, ",\n                 from %s:%"FMTSZ"u", rpf, iy);
        }
        fputs(":\n", stderr);
    }

    /* f */
#ifdef HAVE_COLOR
    if (main_opt()->color)
        fputs(ACLOCUS, stderr);
#endif    /* HAVE_COLOR */
    fprintf(stderr, "%s:", ep->f);

    /* y, x */
    if (y)
        fprintf(stderr, "%"FMTSZ"u:", y);
    if (main_opt()->diagstyle == 1 && x > 0)
        fprintf(stderr, "%"FMTSZ"u:", x);

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
    if (main_opt()->diagstyle == 1 && x)
        putline(ep);
#ifdef HAVE_COLOR
    else if (main_opt()->color)
        fputs(ACRESET, stderr);
#endif    /* HAVE_COLOR */

    /* macro expanded */
    if (from->type == LMAP_MACRO && (prop[code] & P)) {
        rpf = ep->rpf, y = ep->py;
        from = lmap_mstrip(from->from);
        ep = epos(from, 0, 0, 0, NULL);
        if (ep->rpf != rpf || ep->py != y)
            issue(ep, from, ERR_PP_EXPFROM, ap);
    }

    if (prop[code] & F) {
        cnt = -1;
        EXCEPT_RAISE(err_except);
    }

    if (prop[code] & O)
        err_dline(NULL, 1, ERR_XTRA_ONCEFILE);

    if (t == E && ++cnt >= err_lim && err_lim > 0) {
        assert(prop[ERR_XTRA_ERRLIMIT] & F);
        err_dline(NULL, 1, ERR_XTRA_ERRLIMIT);
    }

    return 1;
}


/*
 *  issues a diagnostic message with lmap_t
 */
int (err_dpos)(const lmap_t *pos, int code, ...)
{
    int r;
    va_list ap;

    if (!pos)
        return 0;

    va_start(ap, code);
    r = issue(epos(pos, 0, 0, 0, NULL), pos, code, ap);
    va_end(ap);

    return r;
}


/*
 *  issues a diagnostic message with two or more lmap_t's
 */
int (err_dmpos)(const lmap_t *pos, int code, ...)
{
    int r;
    va_list ap;
    const lmap_t *p;
    struct epos_t *q = NULL;

    if (!pos)
        return 0;

    va_start(ap, code);
    while ((p = va_arg(ap, const lmap_t *)) != NULL)
        q = epos((pos->type == LMAP_NORMAL)? lmap_mstrip(p): p, 0, 0, 0, q);
    q = epos(pos, 0, 0, 0, q);
    r = issue(q, pos, code, ap);
    va_end(ap);

    return r;
}


/*
 *  issues a diagnostic message with a pointer into in_line
 */
int (err_dline)(const char *p, int n, int code, ...)
{
    int r;
    sz_t wx;
    int dy = 0;
    va_list ap;

    assert(code >= 0 && code < NELEM(prop));

    va_start(ap, code);
    wx = (p)? in_getwx(1, in_line, p, &dy): 0;
    r = issue(epos(NULL, in_py+dy, wx, n, NULL), lmap_from, code, ap);
    va_end(ap);

    return r;
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

/* end of err.c */
