/*
 *  expression for preprocessing
 */

#include <ctype.h>         /* isxdigit, isdigit, tolower */
#include <limits.h>        /* ULONG_MAX */
#include <stddef.h>        /* NULL, size_t */
#include <stdlib.h>        /* ldiv */
#include <string.h>        /* strcmp, strchr, strcpy, strcat, strlen */
#include <cbl/arena.h>     /* ARENA_ALLOC */
#include <cbl/assert.h>    /* assert */
#include <cbl/except.h>    /* except_t, EXCEPT_RAISE, EXCEPT_TRY, EXCEPT_EXCEPT, EXCEPT_END */
#ifdef HAVE_ICONV
#include <errno.h>         /* errno, E2BIG */
#include <string.h>        /* memcpy */
#include <iconv.h>         /* iconv */
#endif    /* HAVE_ICONV */

#include "../src/common.h"
#include "../src/err.h"
#include "../src/main.h"
#include "lex.h"
#include "lxl.h"
#include "mcr.h"
#include "strg.h"
#include "util.h"
#include "expr.h"

/* locus for diagnostics */
#define PPOS() ((mcr_mpos)? mcr_mpos: &lex_cpos)

/* issues diagnostics with proper locus */
#define issue(c, s) (err_issuep(PPOS(), c, s))

/* checks if character constant */
#define ISCCON(p) (p[0] == '\'' || (p[0] == 'L' && p[1] == '\''))

/* max/min of s/uint_t on the target;
   ASSUMPTION: 2sC for signed integers assumed */
#define SMAX ((sint_t)ONES(PPINT_BYTE*TG_CHAR_BIT - 1))
#define SMIN (-SMAX-1)
#define UMAX ((uint_t)ONES(PPINT_BYTE*TG_CHAR_BIT))

/* max/min of signed/unsigned char on the target;
   ASSUMPTION: 2sC for signed integers assumed */
#define SCMAX ((sint_t)ONES(TG_CHAR_BIT - 1))
#define SCMIN (-SCMAX-1)
#define UCMAX ((uint_t)ONES(TG_CHAR_BIT))

/* mimics integer conversions on the target;
   ASSUMPTION: 2sC for signed integers assumed;
   ASSUMPTION: signed integers are compatible with unsigned ones on the host */
#define CROPS(n)  ((sint_t)((CROPU(n) > SMAX)? (~UMAX)|CROPU(n): CROPU(n)))
#define CROPSC(n) ((sint_t)((CROPUC(n) > SCMAX)? (~UCMAX)|CROPUC(n): CROPUC(n)))
#define CROPU(n)  (((uint_t)(n)) & UMAX)
#define CROPUC(n) (((uint_t)(n)) & UCMAX)


/* precedence of operators */
static char prec[] = {
#define xx(a, b, c, d, e, f, g, h) c,
#define yy(a, b, c, d, e, f, g, h) c,
#include "../src/xtoken.h"
};

#ifdef HAVE_ICONV
static int endian = 1;                        /* for LITTLE from common.h */
#endif    /* HAVE_ICONV */
static lex_t *pushback;                       /* push-back buffer for a token */
static const except_t invexpr =
                 { "invalid expression" };    /* exception for invalid expression */
static int silent;                            /* positive in unevaluated (sub-)expressions */
static int level;                             /* nesting levels of parenthesized expressions */


/* internal functions referenced forwardly */
static expr_t *expr(lex_t **, int);


/*
 *  reads and prepares a token with expanding macros
 */
static lex_t *preptok(void)
{
    lex_t *t;

    if (pushback) {
        t = pushback;
        pushback = NULL;
    } else
        t = lxl_next();

    while (t->id == LEX_ID && !t->blue && mcr_expand(t, &lex_cpos))
        t = lxl_next();

    if (t->id == LEX_ID && strcmp(t->rep, "defined") == 0 && mcr_mpos)
        issue(ERR_PP_DEFFROMMCR, NULL);

    return t;
}


/*
 *  returns the next non-space token
 */
static lex_t *nextnsp(void)
{
    lex_t *t;

    while ((t=preptok())->id == LEX_SPACE)
        continue;

    return t;
}


/*
 *  returns a string representation of a token
 */
static const char *name(const lex_t *t)
{
    assert(t);

    switch(t->id) {
        case LEX_SPACE:
            return "whitespace";
        case LEX_NEWLINE:
            return "end of line";
        case LEX_EOI:
            assert(!"invalid token -- should never reach here");
            return "end of input";
        default:
            return t->rep;
    }
}


/*
 *  adds signed integers after checking overflow;
 *  ASSUMPTION: overflow from addition is benign
 */
static sint_t add(sint_t l, sint_t r, const lex_pos_t *ppos)
{
    int cond = (l == 0 || r == 0 ||
                (l < 0 && r > 0) ||
                (l > 0 && r < 0) ||
                (l < 0 && r < 0 && l >= SMIN-r) ||
                (l > 0 && r > 0 && l <= SMAX-r));

    assert(ppos);

    if (!cond && !silent)
        err_issuep(ppos, ERR_PP_OVFCONST);

    return CROPS(l + r);
}


/*
 *  subtracts signed integers after checking overflow;
 *  ASSUMPTION: overflow from subtraction is benign
 */
static sint_t sub(sint_t l, sint_t r, const lex_pos_t *ppos)
{
    int cond = (l == 0 || r == 0 ||
                (l < 0 && r < 0) ||
                (l > 0 && r > 0) ||
                (l < 0 && r > 0 && l >= SMIN+r) ||
                (l > 0 && r < 0 && l <= SMAX+r));

    assert(ppos);

    if (!cond && !silent)
        err_issuep(ppos, ERR_PP_OVFCONST);

    return CROPS(l - r);
}


/*
 *  multiplies signed integers after checking overflow;
 *  ASSUMPTION: 2sC for signed integers assumed;
 *  ASSUMPTION: overflow from multipication is benign;
 *  ASSUMPTION: signed integers can be checked with ldiv()
 */
static sint_t mul(sint_t l, sint_t r, const lex_pos_t *ppos)
{
    int cond = (!(l == -1 && r == SMIN) &&
                !(l == SMIN && r == -1) &&
                ((l == 0 || r == 0) ||
                 (l < 0 && r < 0 && l >= ldiv(SMAX, r).quot) ||
                 (l < 0 && r > 0 && l >= ldiv(SMIN, r).quot) ||
                 (l > 0 && r < 0 && (r == -1 || l <= ldiv(SMIN, r).quot)) ||
                 (l > 0 && r > 0 && l <= SMAX/r)));

    assert(ppos);

    if (!cond && !silent)
        err_issuep(ppos, ERR_PP_OVFCONST);

    return ((l == -1 && r == SMIN) || (l == SMIN && r == -1))? SMIN: CROPS(l * r);
}


/*
 *  divides signed integers after checking overflow;
 *  ASSUMPTION: 2sC for signed integers assumed
 */
static sint_t mdiv(sint_t l, sint_t r, int op, const lex_pos_t *ppos)
{
    int cond;

    assert(ppos);

    if (r == 0) {
        if (!silent)
            err_issuep(ppos, ERR_PP_DIVBYZERO);
        return 0;
    }

    cond = !(l == SMIN && r == -1);
    if (!cond && !silent)
        err_issuep(ppos, ERR_PP_OVFCONST);

    if (op == '/') {
        return (l == SMIN && r == -1)? SMIN: l / r;
    } else {
        assert(op == '%');
        return (l == SMIN && r == -1)? 0: l % r;
    }
}


/*
 *  converts the type of an expression result to unsigned type;
 *  note that the value argument may be modified;
 *  ASSUMPTION: signed integers are compatible with unsigned ones on both
 */
static expr_t *castu(expr_t *r, const lex_pos_t *ppos)
{
    assert(r);
    assert(ppos);

    if (r->type == EXPR_TS) {
        if (r->u.s < 0) {
            if (!silent)
                err_issuep(ppos, ERR_PP_NEGTOUNSIGN, NULL);
            r->u.u = CROPU(r->u.s);
        }
        r->type = EXPR_TU;
    }

    return r;
}


/*
 *  generates a new value for signed results
 */
static expr_t *newrs(sint_t v)
{
    expr_t *r = ARENA_ALLOC(strg_line, sizeof(*r));
    r->type = EXPR_TS;
    r->u.s = CROPS(v);

    return r;
}


/*
 *  generates a new value for unsigned results
 */
static expr_t *newru(uint_t v)
{
    expr_t *r = ARENA_ALLOC(strg_line, sizeof(*r));
    r->type = EXPR_TU;
    r->u.u = CROPU(v);

    return r;
}


/*
 *  recognizes an integer constant;
 *  ASSUMPTION: unsigned long on the host can represent all unsigned integers on the target
 */
static expr_t *icon(const char *p)
{
    uint_t n;
    int ovf, err, d, t;
    const char *q = p, *hex = "0123456789abcdef";

    assert(p);

    t = EXPR_TS;
    n = err = ovf = 0;
    if (*p == '0' && (p[1] == 'x' || p[1] == 'X')) {    /* 0x */
        p++;    /* skips 0 */
        while (isxdigit(*(unsigned char *)++p)) {
            d = strchr(hex, tolower(*(unsigned char *)p)) - hex;
            if (n & ~(UMAX >> 4))
                ovf = 1;
            else
                n = (n << 4) + d;
        }
    } else {    /* 0 or other digits */
        int b = (*p == '0')? 8: 10;
        while (isdigit(*(unsigned char *)p)) {
            d = *p++ - '0';
            if (b == 8) {
                if (*p == '8' || *p == '9')
                    err = 1;
                if (n & ~(UMAX >> 3))
                    ovf = 1;
                else
                    n = (n << 3) + d;
            } else {
                if (n > (UMAX - d) / 10)
                    ovf = 1;
                else
                    n = n * 10 + d;
            }
        }
    }

    if (((p[0] == 'u' || p[0] == 'U') && (p[1] == 'l' || p[1] == 'L')) ||
        ((p[0] == 'l' || p[0] == 'L') && (p[1] == 'u' || p[1] == 'U'))) {
        t = EXPR_TU;
        p += 2;
    } else if (*p == 'u' || *p == 'U') {
        t = EXPR_TU;
        p++;
    } else if (*p == 'l' || *p == 'L')
        p++;
    else if (*p == '.' || *p == 'e') {
        issue(ERR_PP_ILLOP, "floating-point constant");
        EXCEPT_RAISE(invexpr);
        /* code below never runs */
        return newrs(0);
    }

    if (*p != '\0') {
        issue(ERR_PP_PPNUMBER, q);
        EXCEPT_RAISE(invexpr);
        /* code below never runs */
    } else if (ovf)
        issue(ERR_PP_OVFCONST, NULL);

    return (n > SMAX || t == EXPR_TU)? newru(n): newrs(n);
}


/*
 *  recognizes and converts a character constant;
 *  ASSUMPTION: int represents all small integers
 */
static expr_t *ccon(const char *p)
{
    int w;
    uint_t c = 0;

    assert(p);

    if (*p == 'L') {
        w = 1;
        p++;
    } else
        w = 0;

    assert(*p == '\'');
    p++;    /* skips ' */

    switch(*p) {
        case '\'':    /* empty; diagnosed elsewhere */
        case '\0':    /* unclosed; diagnosed elsewhere */
            return newrs(0);
        case '\\':    /* escape sequences */
            p++;    /* skips \ */
            /* unsigned short is also treated as uint_t for simplification */
            assert(UMAX >= UCMAX);
            assert(ULONG_MAX >= UMAX);
            c = lex_bs(&p, (!w)? UCMAX: (main_opt()->wchart == 1)? UMAX: SMAX, PPOS(),
                       "expression");
            break;
        default:    /* ordinary characters */
#ifdef HAVE_ICONV
            if (w && main_ntow) {
                uint_t d = 0;

                /* only the first character is converted to simplify code */
                const char *q = p;
                size_t ilenv, olenv = sizeof(d);
                char *ibuf = (char *)p, *obuf = (char *)&d;

                do {
                    q++;
                } while(!FIRSTUTF8(*q));
                ilenv = q - p;

                /* although the first call to iconv() writes nothing for most (if not all) wide
                   character encodings, done to mimic compiler proper; see lex_scon() */
                if ((errno = 0, iconv(*main_ntow, NULL, NULL, &obuf, &olenv) == (size_t)-1) ||
                    (errno = 0, iconv(*main_ntow, &ibuf, &ilenv, &obuf, &olenv) == (size_t)-1)) {
                    issue((errno == E2BIG)? ERR_PP_WIDENOTFIT: ERR_PP_CONVFAIL, NULL);
                    return newrs(0);
                }
                p = ibuf;

                if (LITTLE != main_opt()->little_endian)
                    CHGENDIAN(d, sizeof(d));
                memcpy(&c+((main_opt()->little_endian)? 0: sizeof(c)-PPINT_BYTE), &d, PPINT_BYTE);
            } else
#endif    /* HAVE_ICONV */
                c = *p++;
            break;
    }

    if (*p != '\'')
        issue(ERR_PP_LARGECHAR, NULL);

    /* unsigned short is also treated as uint_t for simplification */
    if (w)
        return (main_opt()->wchart == 1)? newru(c): newrs(c);
    else
        return (main_opt()->uchar)? newru(CROPUC(c)): newrs(CROPSC(c));
}


/*
 *  parses a primary expression
 */
static expr_t *prim(lex_t **pt)
{
    expr_t *r;
    const char *p;

    assert(pt);
    assert(*pt);

    p = (*pt)->rep;
    switch((*pt)->id) {
        case LEX_PPNUM:
            r = icon(p);
            break;
        case LEX_SCON:
            if (ISCCON(p))
                r = ccon(p);
            else {
                issue(ERR_PP_ILLOP, "string literal");
                EXCEPT_RAISE(invexpr);
                /* code below never runs */
            }
            break;
        default:
            if ((*pt)->id == LEX_NEWLINE)
                err_issuep(PPOS(), ERR_PP_EXPRERR, "operand", name(*pt));
            else
                issue(ERR_PP_ILLEXPR, NULL);
            EXCEPT_RAISE(invexpr);
            /* code below never runs */
            /* no break */
        case LEX_ID:
            if ((*pt)->id == LEX_ID && strcmp((*pt)->rep, "defined") == 0) {
                int paren = 0;

                assert(!pushback);
                *pt = skip(lxl_next(), lxl_next);
                if ((*pt)->id == '(') {
                    *pt = skip(lxl_next(), lxl_next);
                    paren = 1;
                }
                if ((*pt)->id != LEX_ID) {
                    issue(ERR_PP_NODEFID, NULL);
                    EXCEPT_RAISE(invexpr);
                    /* code below never runs */
                    return newrs(0);
                } else {
                    r = newrs(mcr_redef((*pt)->rep));
                    if (paren && (*pt = skip(lxl_next(), lxl_next))->id != ')') {
                        issue(ERR_PP_NODEFRPAREN, NULL);
                        EXCEPT_RAISE(invexpr);
                        /* code below never runs */
                        return newrs(0);
                    }
                }
            } else {
                issue(ERR_PP_EXPRUNDEFID, (*pt)->rep);
                r = newrs(0);
            }
            break;
    }
    *pt = nextnsp();

    return r;
}


/*
 *  parses a postfix expression
 */
static expr_t *postfix(lex_t **pt, expr_t *l)
{
    assert(pt);
    assert(*pt);

    for (;;)
        switch((*pt)->id) {
            case LEX_INCR:    /* ++ */
            case LEX_DECR:    /* -- */
            case '[':
            case '(':
            case '.':
            case LEX_DEREF:    /* -> */
                issue(ERR_PP_ILLOP, (*pt)->rep);
                EXCEPT_RAISE(invexpr);
                /* code below never runs */
                *pt = nextnsp();
                /* no break */
            default:
                return l;
        }

    /* assert(!"invalid control flow -- should never reach here");
       return l; */
}


/*
 *  parses a unary expression;
 *  ASSUMPTIONS: overflow from negation is benign;
 *  ASSUMPTIONS: the target has no signed zero
 */
static expr_t *unary(lex_t **pt)
{
    expr_t *l;
    lex_pos_t pos;

    assert(pt);
    assert(*pt);

    switch((*pt)->id) {
        case '*':
        case '&':
        case LEX_INCR:    /* ++ */
        case LEX_DECR:    /* -- */
            issue(ERR_PP_ILLOP, (*pt)->rep);
            EXCEPT_RAISE(invexpr);
            /* code below never runs */
            *pt = nextnsp();
            l = newrs(0);
            break;
        case '+':
            *pt = nextnsp();
            l = unary(pt);
            break;
        case '-':
            pos = *PPOS();
            *pt = nextnsp();
            l = unary(pt);
            if (l->type == EXPR_TS)
                l->u.s = CROPS(-l->u.s);
            else {
                if (!silent)
                    err_issuep(&pos, ERR_PP_NEGUNSIGNED, NULL);
                l->u.u = CROPU(-l->u.u);
            }
            break;
        case '~':
            *pt = nextnsp();
            l = unary(pt);
            if (l->type == EXPR_TS)
                l->u.s = CROPS(~l->u.s);
            else
                l->u.u = CROPU(~l->u.u);
            break;
        case '!':
            *pt = nextnsp();
            l = unary(pt);
            l->u.u = (l->u.u == 0);
            break;
        case '(':
            if (level++ == TL_PARENE_STD) {
                err_issuep(PPOS(), ERR_PP_MANYPE);
                err_issuep(PPOS(), ERR_PP_MANYPESTD, (long)TL_PARENE_STD);
            }
            *pt = nextnsp();
            l = postfix(pt, expr(pt, ')'));
            level--;
            break;
        default:
            l = postfix(pt, prim(pt));
            break;
    }

    return l;
}


/*
 *  evaluates binary operators for unsigned results;
 *  ASSUMPTIONS: over-shift or negative shift is benign
 */
static expr_t *evalbinu(int op, expr_t *l, expr_t *r, const lex_pos_t *ppos)
{
    uint_t lval, rval;

    assert(l);
    assert(r);
    assert(ppos);

    switch(op) {
        /* usual arithmetic conversion; result has signed type */
        case LEX_EQEQ:    /* == */
        case LEX_NEQ:    /* != */
        case LEX_LEQ:    /* <= */
        case LEX_GEQ:    /* >= */
        case '<':
        case '>':
            lval = castu(l, ppos)->u.u;    /* l's type modified */
            rval = castu(r, ppos)->u.u;
            switch(op) {
                case LEX_EQEQ:    /* == */
                    l->u.u = (lval == rval);
                    break;
                case LEX_NEQ:    /* != */
                    l->u.u = (lval != rval);
                    break;
                case LEX_LEQ:    /* <= */
                    l->u.u = (lval <= rval);
                    break;
                case LEX_GEQ:    /* >= */
                    l->u.u = (lval >= rval);
                    break;
                case '<':
                    l->u.u = (lval < rval);
                    break;
                case '>':
                    l->u.u = (lval > rval);
                    break;
                default:
                    assert(!"invalid binary operator -- should never reach here");
                    break;
            }
            l->type = EXPR_TS;
            break;

        /* result has l's type */
        case LEX_RSHFT:
            if (r->type == EXPR_TS) {
                if ((r->u.s < 0 || r->u.s >= sizeof(sint_t)*TG_CHAR_BIT) && !silent)
                    err_issuep(ppos, ERR_PP_OVERSHIFTS, (long)r->u.s);
                l->u.u = CROPU(l->u.u >> r->u.s);
            } else {
                if (r->u.u >= sizeof(uint_t)*TG_CHAR_BIT && !silent)
                    err_issuep(ppos, ERR_PP_OVERSHIFTU, (unsigned long)r->u.u);
                l->u.u = CROPU(l->u.u >> r->u.u);
            }
            break;
        case LEX_LSHFT:
            if (r->type == EXPR_TS) {
                if ((r->u.s < 0 || r->u.s >= sizeof(sint_t)*TG_CHAR_BIT) && !silent)
                    err_issuep(ppos, ERR_PP_OVERSHIFTS, (long)r->u.s);
                l->u.u = CROPU(l->u.u << r->u.s);
            } else {
                if (r->u.u >= sizeof(uint_t)*TG_CHAR_BIT && !silent)
                    err_issuep(ppos, ERR_PP_OVERSHIFTU, (unsigned long)r->u.u);
                l->u.u = CROPU(l->u.u << r->u.u);
            }
            break;

        /* result has type from usual arithmetic conversion */
        case '|':
        case '^':
        case '&':
        case '+':
        case '-':
        case '*':
        case '/':
        case '%':
            lval = castu(l, ppos)->u.u;    /* l's type modified */
            rval = castu(r, ppos)->u.u;
            switch(op) {
                case '|':
                    l->u.u = lval | rval;
                    break;
                case '^':
                    l->u.u = lval ^ rval;
                    break;
                case '&':
                    l->u.u = lval & rval;
                    break;
                case '+':
                    l->u.u = CROPU(lval + rval);
                    break;
                case '-':
                    l->u.s = CROPU(lval - rval);
                    break;
                case '*':
                    l->u.s = CROPU(lval * rval);
                    break;
                case '/':
                case '%':
                    if (rval == 0) {
                        if (!silent)
                            err_issuep(ppos, ERR_PP_DIVBYZERO);
                        l->u.u = 0;
                    } else
                        l->u.u = CROPU((op == '/')? lval / rval: lval % rval);
                    break;
                default:
                    assert(!"invalid binary operator -- should never reach here");
                    break;
            }
            break;
        default:
            assert(!"invalid binary operator -- should never reach here");
            break;
    }

    return l;
}


/*
 *  evaluates binary operators for signed results;
 *  no conversion occurs, which simplifies code;
 *  ASSUMPTIONS: bitwise operators do the same as on the target;
 *  ASSUMPTIONS: over-shift or negative shift is benign
 */
static expr_t *evalbins(int op, expr_t *l, expr_t *r, const lex_pos_t *ppos)
{
    assert(l);
    assert(r);
    assert(ppos);

    switch(op) {
        case '|':
            l->u.s = CROPS(l->u.s | r->u.s);
            break;
        case '^':
            l->u.s = CROPS(l->u.s ^ r->u.s);
            break;
        case '&':
            l->u.s = CROPS(l->u.s & r->u.s);
            break;
        case LEX_EQEQ:    /* == */
            l->u.s = (l->u.s == r->u.s);
            break;
        case LEX_NEQ:    /* != */
            l->u.s = (l->u.s != r->u.s);
            break;
        case LEX_LEQ:    /* <= */
            l->u.s = (l->u.s <= r->u.s);
            break;
        case LEX_GEQ:    /* >= */
            l->u.s = (l->u.s >= r->u.s);
            break;
        case '<':
            l->u.s = (l->u.s < r->u.s);
            break;
        case '>':
            l->u.s = (l->u.s > r->u.s);
            break;
        case LEX_RSHFT:    /* >> */
            {
                sint_t n;

                if (l->u.s < 0 && !silent)
                    err_issuep(ppos, ERR_PP_RSHIFTNEG);
                if (main_opt()->logicshift)
                    return evalbinu(op, l, r, ppos);
                if (r->type == EXPR_TS) {
                    if ((r->u.s < 0 || r->u.s >= sizeof(sint_t)*TG_CHAR_BIT) && !silent)
                        err_issuep(ppos, ERR_PP_OVERSHIFTS, (long)r->u.s);
                    n = CROPS(l->u.s >> r->u.s);
                    if (r->u.s >= 0 && l->u.s < 0 && n >= 0)
                        n |= ~(~0UL >> r->u.s);
                } else {
                    if (r->u.u >= sizeof(uint_t)*TG_CHAR_BIT && !silent)
                        err_issuep(ppos, ERR_PP_OVERSHIFTU, (unsigned long)r->u.u);
                    n = CROPS(l->u.s >> r->u.u);
                    if (l->u.s < 0 && n >= 0)
                        n |= ~(~0UL >> r->u.u);
                }
                /* cannot assert(l->u.s >= 0 || n < 0) */
                l->u.s = n;
            }
            break;
        case LEX_LSHFT:    /* << */
            if (l->u.s < 0 && !silent)
                err_issuep(ppos, ERR_PP_LSHIFTNEG);
            if (r->type == EXPR_TS) {
                if ((r->u.s < 0 || r->u.s >= sizeof(sint_t)*TG_CHAR_BIT ||
                     (l->u.s && r->u.s >= sizeof(sint_t)*TG_CHAR_BIT-1)) && !silent)
                    err_issuep(ppos, ERR_PP_OVERSHIFTS, (long)r->u.s);
                else if (l->u.s >= 0)
                    mul(l->u.s, ((sint_t)1) << r->u.s, ppos);
                l->u.s = CROPS(l->u.s << r->u.s);
            } else {
                if ((r->u.u >= sizeof(uint_t)*TG_CHAR_BIT ||
                     (l->u.s && r->u.u >= sizeof(uint_t)*TG_CHAR_BIT-1)) && !silent)
                    err_issuep(ppos, ERR_PP_OVERSHIFTU, (unsigned long)r->u.u);
                else if (l->u.s >= 0)
                    mul(l->u.s, ((sint_t)1) << r->u.u, ppos);
                l->u.s = CROPS(l->u.s << r->u.u);
            }
            break;
        case '+':
            l->u.s = add(l->u.s, r->u.s, ppos);
            break;
        case '-':
            l->u.s = sub(l->u.s, r->u.s, ppos);
            break;
        case '*':
            l->u.s = mul(l->u.s, r->u.s, ppos);
            break;
        case '/':
        case '%':
            l->u.s = mdiv(l->u.s, r->u.s, op, ppos);
            break;
        default:
            assert(!"invalid binary operator -- should never reach here");
            break;
    }

    return l;
}


/*
 *  returns the result type of binary/tenary operators;
 *  ASSUMPTION: the target has no signed zero
 */
static int type(int op, const expr_t *l, const expr_t *r)
{
    assert(l);
    assert(r);

    if (op == LEX_RSHFT || op == LEX_LSHFT)
        return l->type;
    else
        return (l->type == EXPR_TU || r->type == EXPR_TU)? EXPR_TU: EXPR_TS;
}


/*
 *  parses a binary expression
 */
static expr_t *bin(lex_t **pt, int k)
{
    static expr_t *(*eval[])() = { evalbins, evalbinu };

    int k1;
    int tid;
    expr_t *l, *r;

    assert(pt);
    assert(*pt);

    l = unary(pt);
    tid = (*pt)->id;
    for (k1 = prec[tid]; k1 >= k; k1--)
        while (prec[tid] == k1) {
            lex_t *t;
            lex_pos_t pos = *PPOS();

            t = preptok();
            pushback = t;
            if (t->id == '=')
                break;
            *pt = nextnsp();
            r = bin(pt, k1+1);    /* no side effect; can evaluate both */
            l = eval[type(tid, l, r)](tid, l, r, &pos);
            tid = (*pt)->id;
        }

    return l;
}


/*
 *  parses a logical-AND expression;
 *  separated from bin() to implement short-circuit
 */
static expr_t *and(lex_t **pt)
{
    expr_t *l;
    int os = silent;

    assert(pt);
    assert(*pt);

    l = bin(pt, 6);
    if ((*pt)->id == LEX_ANDAND) {
        if (!l->u.u)
            silent++;
        do {
            *pt = nextnsp();
            if (!bin(pt, 6)->u.u)
                silent++;
        } while((*pt)->id == LEX_ANDAND);
        l->type = EXPR_TS;
        if (silent != os) {
            silent = os;
            l->u.u = 0;
        } else
            l->u.u = 1;
    }

    return l;
}


/*
 *  parses a logical-OR expression;
 *  separated from bin() to implement short-circuit
 */
static expr_t *or(lex_t **pt)
{
    expr_t *l;
    int os = silent;

    assert(pt);
    assert(*pt);

    l = and(pt);
    if ((*pt)->id == LEX_OROR) {
        if (l->u.u)
            silent++;
        do {
            *pt = nextnsp();
            if (and(pt)->u.u)
                silent++;
        } while ((*pt)->id == LEX_OROR);
        l->type = EXPR_TS;
        if (silent != os) {
            silent = os;
            l->u.u = 1;
        } else
            l->u.u = 0;
    }

    return l;
}


/*
 *  parses a conditional expression
 */
static expr_t *cond(lex_t **pt)
{
    expr_t *c, *l, *r;
    lex_pos_t lpos, rpos;

    assert(pt);
    assert(*pt);

    c = or(pt);
    if ((*pt)->id == '?') {
        *pt = nextnsp();
        /* no side effect, thus can evaluate both */
        lpos = *PPOS();
        if (c->u.u)
            l = expr(pt, ':');
        else {
            silent++;
            l = expr(pt, ':');
            silent--;
        }
        rpos = *PPOS();
        if (c->u.u) {
            silent++;
            r = cond(pt);
            silent--;
        } else
            r = cond(pt);
        if (type('?', l, r) == EXPR_TU) {
            l = castu(l, &lpos);
            r = castu(r, &rpos);
        }
        return (c->u.u)? l: r;
    }

    return c;
}


/*
 *  parses an assignment expression
 */
static expr_t *asgn(lex_t **pt)
{
    int tid;
    expr_t *r;
    char caop[4];    /* e.g., <<= */

    assert(pt);
    assert(*pt);

    r = cond(pt);
    tid = (*pt)->id;
    if (tid == '=') {
        issue(ERR_PP_ILLOP, "=");
        EXCEPT_RAISE(invexpr);
        /* code below never runs */
        *pt = nextnsp();
    } else if ((prec[tid] >= 6 && prec[tid] <= 8) || (prec[tid] >= 11 && prec[tid] <= 13)) {
        const char *p = (*pt)->rep;
        *pt = nextnsp();
        if ((*pt)->id == '=') {
            assert(strlen(p) <= 2);
            strcpy(caop, p);
            strcat(caop, "=");
            issue(ERR_PP_ILLOP, caop);
        } else
            issue(ERR_PP_ILLEXPR, NULL);
        EXCEPT_RAISE(invexpr);
        /* code below never runs */
    }

    return r;
}


/*
 *  parses an expression
 */
static expr_t *expr(lex_t **pt, int tid)
{
    expr_t *r;
    char s[4] = "` '";

    assert(pt);
    assert(*pt);

    r = asgn(pt);
    if ((*pt)->id == ',') {
        if (main_opt()->std == 1 || !silent)
            issue(ERR_PP_ILLOPW, ",");
        /* accepts , */
        do {
            *pt = nextnsp();
            if ((*pt)->id == LEX_NEWLINE)
                break;
            r = asgn(pt);
        } while((*pt)->id == ',');
    }
    if (tid) {
        if ((*pt)->id == tid)
            *pt = nextnsp();
        else {
            s[1] = tid;
            err_issuep(PPOS(), ERR_PP_EXPRERR, s, name(*pt));
            EXCEPT_RAISE(invexpr);
            /* code below never runs */
        }
    }

    return r;
}


/*
 *  starts parsing a controlling expression;
 *  constitutes a separate function to handle exceptions;
 *  a caller has to skip to the next newline
 */
expr_t *(expr_start)(lex_t **pt, const char *k)
{
    expr_t *r;

    assert(pt);
    assert(*pt);
    assert(sizeof(sint_t) >= PPINT_BYTE);

    level = silent = 0;
    pushback = *pt;
    *pt = nextnsp();
    if ((*pt)->id == LEX_NEWLINE)
        err_issuep(&lex_cpos, ERR_PP_NOIFEXPR, k);
    else {
        EXCEPT_TRY
            r = expr(pt, 0);
            if ((*pt)->id == LEX_NEWLINE)
                EXCEPT_RETURN r;
            else {
                switch((*pt)->id) {
                    case ')':    /* closing paren */
                        issue(ERR_PP_NOEXPRLPAREN, NULL);
                        break;
                    /* operands */
                    case LEX_SCON:
                        if (!ISCCON((*pt)->rep)) {
                            issue(ERR_PP_ILLOP, "string literal");
                            break;
                        }
                        /* no break */
                    case LEX_PPNUM:
                    case LEX_ID:
                        err_issuep(PPOS(), ERR_PP_EXPRERR, "operator", name(*pt));
                        break;
                    default:
                        issue(ERR_PP_ILLEXPR, NULL);
                        break;
                }
            }
        EXCEPT_EXCEPT(invexpr)
            /* nothing to do */
        EXCEPT_END
    }

    return newrs(0);
}

/* end of expr.c */
