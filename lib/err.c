/*
 *  error handling
 */

#include <stdarg.h>        /* va_list, va_start, va_end */
#include <stddef.h>        /* NULL*/
#include <stdio.h>         /* stderr, fprintf, putc, fputs */
#include <string.h>        /* strlen */
#include <cbl/arena.h>     /* ARENA_ALLOC */
#include <cbl/assert.h>    /* assert */
#include <cbl/except.h>    /* except_t, EXCEPT_RAISE */

#include "common.h"
#include "in.h"
#include "inc.h"
#include "lmap.h"
#include "main.h"
#include "strg.h"
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
 *  composes a locus for diagnostics;
 *  intended to be consumed immediately
 */
static struct epos_t *epos(const lmap_t *h, sz_t py, sz_t wx, int n, struct epos_t *q)
{
    static struct epos_t ep;

    struct epos_t *p = (q)? ARENA_ALLOC(strg_line, sizeof(*p)): &ep;

    if (h) {    /* token locus */
        while (h->type == LMAP_MACRO)
            h = h->u.m;
        assert(h->type == LMAP_NORMAL);
        if (n == 0) {    /* token itself */
            py = h->u.n.py;
            p->wx = h->u.n.wx;
            p->dy = h->u.n.dy;
            p->dx = h->u.n.dx;
        } else {    /* after token */
            py = h->u.n.py + h->u.n.dy;
            p->wx = h->u.n.dx;
            p->dy = 0;
            p->dx = p->wx + n;
        }
    } else {    /* lmap_from + locus */
        h = lmap_from;
        p->wx = wx;
        p->dy = 0;
        p->dx = wx + n;
    }
    assert(py > 0);
    assert(p->dy > 0 || p->dx > p->wx);

    h = lmap_ninfo(h);
    p->f = (h->type == LMAP_LINE)? h->u.l.f: NULL;
    p->y = py + h->u.i.yoff;    /* cis */

    h = lmap_pinfo(h);
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
                else if (c == eposs[i]->dx) {
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
 *  prints a diagnostic message with custom format characters
 */
static void fmt(const char *s, va_list ap)
{
    char c;

    assert(s);

    while ((c = *s++) != '\0') {
        if (c == '%')
            switch(c = *s++) {
                case 'c':    /* char */
                    putc(va_arg(ap, int), stderr);
                    break;
                case 'd':    /* long */
                    fprintf(stderr, "%ld", va_arg(ap, long));
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


#define showx() (main_opt()->diagstyle == 1 && x)

/*
 *  issues a diagnostic message
 */
static void issue(struct epos_t *ep, const lmap_t *from, int code, va_list ap)
{
    int t;
    sz_t y, x;
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

    pos = lmap_pinfo((from->type == LMAP_MACRO)? from->u.m: from);
    if (!(prop[code] & F) && (t != E && (nowarn[code] ||
                                         (t != N && pos->type == LMAP_INC && pos->u.i.system))))
        return;
    if ((prop[code] & W) && !main_opt()->addwarn && !main_opt()->std)    /* additional warning */
        return;
    if ((prop[code] & (A|B|C)) &&
        !(((prop[code] & A) && main_opt()->std == 1) ||      /* C90 warning */
          ((prop[code] & B) && main_opt()->std == 2) ||      /* C99 warning */
          ((prop[code] & C) && main_opt()->std == 3)))       /* C1X warning */
        return;
    if (prop[code] & O)
        nowarn[code] = 1;

    /* #include chain */
    if (pos->type == LMAP_INC && !pos->u.i.printed) {
#ifdef HAVE_COLOR
        if (main_opt()->color)
            fputs(ACLOCUS, stderr);
#endif    /* HAVE_COLOR */
        ((lmap_t *)pos)->u.i.printed = 1;
        assert(pos->from->type == LMAP_NORMAL);
        y = pos->from->u.n.py;
        pos = lmap_ninfo(pos->from);
        rpf = pos->u.i.f;      /* cis */
        y += pos->u.i.yoff;    /* cis */
        fprintf(stderr, "In file included from %s:%"FMTSZ"u", rpf, y);
        while (pos->type == LMAP_INC) {
            assert(pos->from->type == LMAP_NORMAL);
            y = pos->from->u.n.py;
            pos = lmap_ninfo(pos->from);
            rpf = pos->u.i.f;      /* cis */
            y += pos->u.i.yoff;    /* cis */
            fprintf(stderr, "\n                 from %s:%"FMTSZ"u", rpf, y);
        }
        fputs(":\n", stderr);
    }

    /* f */
#ifdef HAVE_COLOR
    if (main_opt()->color)
        fputs(ACLOCUS, stderr);
#endif    /* HAVE_COLOR */
    fprintf(stderr, "%s:", (*ep->f)? ep->f: "<stdin>");

    /* y, x */
    if (y)
        fprintf(stderr, "%"FMTSZ"u:", y);
    if (showx())
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
        putc('\n', stderr);
    }

    /* source line */
    if (main_opt()->diagstyle == 1 && x)
        putline(ep);
#ifdef HAVE_COLOR
    else if (main_opt()->color)
        fputs(ACRESET, stderr);
#endif    /* HAVE_COLOR */

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
}

#undef showx


/*
 *  issues a diagnostic message with lmap_t
 */
void (err_dpos)(const lmap_t *pos, int code, ...)
{
    va_list ap;

    assert(pos);

    va_start(ap, code);
    issue(epos(pos, 0, 0, 0, NULL), pos, code, ap);
    va_end(ap);
}


/*
 *  issues a diagnostic message with two or more lmap_t's
 */
void (err_dmpos)(const lmap_t *pos, int code, ...)
{
    va_list ap;
    const lmap_t *p;
    struct epos_t *q = NULL;

    assert(pos);

    va_start(ap, code);
    while ((p = va_arg(ap, const lmap_t *)) != NULL)
        q = epos(p, 0, 0, 0, q);
    q = epos(pos, 0, 0, 0, q);
    issue(q, pos, code, ap);
    va_end(ap);
}


/*
 *  issues a diagnostic message with a pointer into in_line
 */
void (err_dline)(const char *p, int n, int code, ...)
{
    sz_t wx;
    int dy = 0;
    va_list ap;

    assert(code >= 0 && code < NELEM(prop));

    va_start(ap, code);
    wx = (p)? in_getwx(1, in_line, p, &dy): 0;
    issue(epos(NULL, in_py+dy, wx, n, NULL), lmap_from, code, ap);
    va_end(ap);
}


/*
 *  issues a diagnostic message at the end of a token
 */
void (err_dafter)(const lmap_t *pos, int code, ...)
{
    va_list ap;

    assert(pos);

    va_start(ap, code);
    issue(epos(pos, 0, 0, 1, NULL), pos, code, ap);
    va_end(ap);
}


/*
 *  issues a diagnostic message at the end of a token with two or more lmap_t's
 */
void (err_dmafter)(const lmap_t *pos, int code, ...)
{
    va_list ap;
    const lmap_t *p;
    struct epos_t *q = NULL;

    assert(pos);

    va_start(ap, code);

    while ((p = va_arg(ap, const lmap_t *)) != NULL)
        q = epos(p, 0, 0, 0, q);
    q = epos(pos, 0, 0, 1, q);
    issue(q, pos, code, ap);
    va_end(ap);
}

/* end of err.c */
