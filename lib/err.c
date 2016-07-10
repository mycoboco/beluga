/*
 *  error handling
 */

#include <stdarg.h>        /* va_list, va_start, va_end */
#include <stddef.h>        /* NULL*/
#include <stdio.h>         /* stderr, fprintf, putc, fputs */
#include <cbl/arena.h>     /* ARENA_ALLOC */
#include <cbl/assert.h>    /* assert */
#include <cbl/except.h>    /* except_t, EXCEPT_RAISE */

#include "common.h"
#include "in.h"
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
    E = 1,      /* error */
    N,          /* note */

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

/* locus for diagnostics */
typedef struct epos_t {
    const char *rpf;              /* resolved physical file name */
    const char *pf;               /* physical file name */
    long py;                      /* physical line # */
    const char *f;                /* nominal file name */
    long y;                       /* nominal line # */
    long wx;                      /* x counted by wcwidth() */
    long n;                       /* range length */
    const struct epos_t *next;    /* next node */
} epos_t;


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


/*
 *  composes a locus for diagnostics
 */
static const epos_t *epos(const lmap_t *h, long py, long wx, long n, const epos_t *q)
{
    static epos_t pos;

    epos_t *p = (q)? ARENA_ALLOC(strg_func, sizeof(*p)): &pos;

    assert(h);
    assert(py > 0);
    assert(wx >= 0);
    assert(n > 0);

    p->wx = (h->type == LMAP_NORMAL)? h->u.n.wx: wx;
    h = lmap_getni(h);
    if (h->type == LMAP_LINE) {
        p->f = h->u.l.f;
        p->y = py + h->u.l.yoff;
    } else
        p->y = py;
    h = lmap_getpi(h);
    p->rpf = h->u.i.rf;
    p->pf = h->u.i.f;
    p->py = py;
    if (!p->f)
        p->f = p->pf;
    p->n = n;
    p->next = q;

    return p;
}


#define SAMELINE(p, q) ((p)->rpf == (q)->rpf && (p)->py == (q)->py)
#define SWAP(p, q)     (ps[4] = (p), (p) = (q), (q) = ps[4])

/*
 *  prints a source line;
 *  ASSUMPTION: different epos_t's do not overlap
 */
static void putline(const epos_t *pos)
{
    int cnt = 1;
    const char *p;
    const epos_t *ps[4], *caret;

    assert(pos);

    p = lmap_flget(pos->rpf, pos->py);
    if (!p)
        return;

    caret = ps[0] = pos;
    if (pos->next && SAMELINE(pos, pos->next))
        ps[cnt] = pos->next, cnt++;
    if (ps[cnt-1]->next && SAMELINE(pos, ps[cnt]))
        ps[cnt] = ps[cnt-1]->next, cnt++;

    if (cnt > 1) {
        if (ps[0]->wx > ps[1]->wx)
            SWAP(ps[0], ps[1]);
        if (cnt > 2) {
            if (ps[0]->wx > ps[2]->wx)
                SWAP(ps[0], ps[2]);
            if (ps[1]->wx > ps[2]->wx)
                SWAP(ps[1], ps[2]);
        }
    }

#ifdef HAVE_COLOR
    if (main_opt()->color)
        fputs(ACRESET"  "ACQUOTE, stderr);
    else
#endif    /* HAVE_COLOR */
        fputs("  ", stderr);
    while (*p != '\0') {
        putc((*p == '\t' || *p == '\v' || *p == '\f' || *p == '\r')? ' ': *p, stderr);
        p++;
    }

#ifdef HAVE_COLOR
    if (main_opt()->color)
        fputs(ACRESET"\n  ", stderr);
#else    /* !HAVE_COLOR */
        fputs("\n  ", stderr);
#endif    /* HAVE_COLOR */
    {
        int i, c;
        for (i=0, c=1; i < cnt; i++) {
            assert(c <= ps[i]->wx);
            while (1) {
                if (c < ps[i]->wx)
                    putc(' ', stderr);
                else if (c == ps[i]->wx) {
#ifdef HAVE_COLOR
                    if (main_opt()->color)
                        fputs((ps[i] == caret)? ACCARET"^": ACCARET"~", stderr);
                    else
#endif    /* HAVE_COLOR */
                        fputs((ps[i] == caret)? "^": "~", stderr);
                } else if (c > ps[i]->wx && c < ps[i]->wx+ps[i]->n)
                    putc('~', stderr);
                else if (c == ps[i]->wx+ps[i]->n) {
#ifdef HAVE_COLOR
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

#undef SAMELINE
#undef SWAP


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
static void issue(const epos_t *pos, int code, va_list ap)
{
    int t;
    unsigned long y, x;

    assert(pos);
    assert(code >= 0 && code < NELEM(msg));
    assert(pos->f);
    assert(msg[code]);

    t = dtype(prop[code]);
    y = (prop[code] & P)? pos->y: 0;
    x = (y == 0)? 0: pos->wx;

    if ((prop[code] & W) && !main_opt()->addwarn && !main_opt()->std)    /* additional warning */
        return;
    if ((prop[code] & (A|B|C)) &&
        !(((prop[code] & A) && main_opt()->std == 1) ||      /* C90 warning */
          ((prop[code] & B) && main_opt()->std == 2) ||      /* C99 warning */
          ((prop[code] & C) && main_opt()->std == 3)))       /* C1X warning */
        return;

    /* f */
#ifdef HAVE_COLOR
    if (main_opt()->color)
        fputs(ACLOCUS, stderr);
#endif    /* HAVE_COLOR */
    fprintf(stderr, "%s:", pos->f);

    /* y, x */
    if (y)
        fprintf(stderr, "%ld:", y);
    if (showx())
        fprintf(stderr, "%ld:", x);

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
        putline(pos);
#ifdef HAVE_COLOR
    else if (main_opt()->color)
        fputs(ACRESET, stderr);
#endif    /* HAVE_COLOR */

    if (prop[code] & F) {
        cnt = -1;
        EXCEPT_RAISE(err_except);
    }
}

#undef showx


/*
 *  issues a diagnostic message with a pointer into in_line
 */
void (err_issuel)(const char *p, int code, ...)
{
    long wx;
    va_list ap;

    assert(code >= 0 && code < NELEM(prop));

    if (p == NULL || (wx = in_getwx(in_line, p)) < 0)
        wx = 0;

    va_start(ap, code);
    issue(epos(lmap_head, in_py, wx, 1, NULL), code, ap);
    va_end(ap);
}

/* end of err.c */
