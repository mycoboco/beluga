/*
 *  expression for preprocessing
 */

#include <ctype.h>         /* isxdigit, isdigit, tolower */
#include <limits.h>        /* CHAR_BIT */
#include <stddef.h>        /* NULL */
#include <stdlib.h>        /* ldiv */
#include <string.h>        /* strcmp, strchr */
#include <cbl/arena.h>     /* ARENA_ALLOC */
#include <cbl/assert.h>    /* assert */
#include <cbl/except.h>    /* except_t, EXCEPT_RAISE, EXCEPT_TRY, EXCEPT_EXCEPT, EXCEPT_END */

#include "clx.h"       /* to use clx_ccon() */
#include "common.h"
#include "err.h"
#include "main.h"
#include "lex.h"
#include "lst.h"
#include "mcr.h"
#include "strg.h"
#include "expr.h"

/* max/min of s/ux_t on the target;
   ASSUMPTION: 2sC for signed integers assumed */
#define SMAX ((sx_t)ONES(PPINT_BYTE*TG_CHAR_BIT - 1))
#define SMIN (-SMAX-1)
#define UMAX ((ux_t)ONES(PPINT_BYTE*TG_CHAR_BIT))

/* mimics integer conversions on the target;
   ASSUMPTION: 2sC for signed integers assumed;
   ASSUMPTION: signed integers are compatible with unsigned ones on the host */
#define CROPS(n)  ((sx_t)((CROPU(n) > SMAX)? (~UMAX)|CROPU(n): CROPU(n)))
#define CROPU(n)  (((ux_t)(n)) & UMAX)


/* operator precedence */
static char prec[] = {
#define xx(a, b, c, d, e, f, g, h) c,
#define kk(a, b, c, d, e, f, g, h) c,
#define yy(a, b, c, d, e, f, g, h) c,
#include "xtoken.h"
};

#ifdef HAVE_ICONV
static int endian = 1;                        /* for LITTLE from common.h */
#endif    /* HAVE_ICONV */
static lex_t *pushback;                       /* push-back buffer for token */
static const except_t invexpr =
                 { "invalid expression" };    /* exception for invalid expression */
static int silent;                            /* positive in unevaluated (sub-)expressions */
static int level;                             /* nesting level of parenthesized expressions */


/* internal functions referenced forwardly */
static expr_t *expr(lex_t **, int, const lmap_t *);


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
        t = lst_nexti();

    while (t->id == LEX_ID && !t->f.blue && mcr_expand(t))
        t = lst_nexti();

    if (t->id == LEX_ID && strcmp(LEX_SPELL(t), "defined") == 0 && t->pos->type == LMAP_MACRO)
        err_dpos(t->pos, ERR_PP_DEFFROMMCR);

    return t;
}


/*
 *  returns the next non-space token
 */
static lex_t *nextnsp(void)
{
    lex_t *t;

    while ((t = preptok())->id == LEX_SPACE)
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
        case LEX_EOI:
            assert(!"invalid token -- should never reach here");
            return "end of input";
        case LEX_SPACE:
            return "whitespace";
        case LEX_NEWLINE:
            return "end of line";
        default:
            return LEX_SPELL(t);
    }

    /* assert(!"invalid control flow -- should never reach here");
       return LEX_SPELL(t); */
}


/*
 *  adds signed integers after checking overflow;
 *  ASSUMPTION: overflow from addition is benign
 */
static sx_t add(expr_t *l, expr_t *r, const lmap_t *pos)
{
    sx_t lv, rv;
    int cond;

    assert(l);
    assert(r);
    assert(pos);

    lv = l->u.s;
    rv = r->u.s;
    cond = (lv == 0 || rv == 0 ||
            (lv < 0 && rv > 0) ||
            (lv > 0 && rv < 0) ||
            (lv < 0 && rv < 0 && lv >= SMIN-rv) ||
            (lv > 0 && rv > 0 && lv <= SMAX-rv));

    if (!cond && !silent)
        err_dmpos(pos, ERR_PP_OVFCONST, lmap_range(l->spos, l->epos),
                  lmap_range(r->spos, r->epos), NULL);

    return CROPS(lv + rv);
}


/*
 *  subtracts signed integers after checking overflow;
 *  ASSUMPTION: overflow from subtraction is benign
 */
static sx_t sub(expr_t *l, expr_t *r, const lmap_t *pos)
{
    sx_t lv, rv;
    int cond;

    assert(l);
    assert(r);
    assert(pos);

    lv = l->u.s;
    rv = r->u.s;
    cond = (lv == 0 || rv == 0 ||
            (lv < 0 && rv < 0) ||
            (lv > 0 && rv > 0) ||
            (lv < 0 && rv > 0 && lv >= SMIN+rv) ||
            (lv > 0 && rv < 0 && lv <= SMAX+rv));

    if (!cond && !silent)
        err_dmpos(pos, ERR_PP_OVFCONST, lmap_range(l->spos, l->epos),
                  lmap_range(r->spos, r->epos), NULL);

    return CROPS(lv - rv);
}


/*
 *  multiplies signed integers after checking overflow;
 *  ASSUMPTION: 2sC for signed integers assumed;
 *  ASSUMPTION: overflow from multiplication is benign;
 *  ASSUMPTION: signed integers can be checked with ldiv()
 */
static sx_t mul(expr_t *l, expr_t *r, const lmap_t *pos)
{
    sx_t lv, rv;
    int cond;

    assert(l);
    assert(r);
    assert(pos);

    lv = l->u.s;
    rv = r->u.s;
    cond = (!(lv == -1 && rv == SMIN) &&
            !(lv == SMIN && rv == -1) &&
            ((lv == 0 || rv == 0) ||
             (lv < 0 && rv < 0 && lv >= ldiv(SMAX, rv).quot) ||
             (lv < 0 && rv > 0 && lv >= ldiv(SMIN, rv).quot) ||
             (lv > 0 && rv < 0 && (rv == -1 || lv <= ldiv(SMIN, rv).quot)) ||
             (lv > 0 && rv > 0 && lv <= SMAX/rv)));

    if (!cond && !silent)
        err_dmpos(pos, ERR_PP_OVFCONST, lmap_range(l->spos, l->epos),
                  lmap_range(r->spos, r->epos), NULL);

    return ((lv == -1 && rv == SMIN) || (lv == SMIN && rv == -1))? SMIN: CROPS(lv * rv);
}


/*
 *  divides signed integers after checking overflow;
 *  ASSUMPTION: 2sC for signed integers assumed
 */
static sx_t mdiv(expr_t *l, expr_t *r, int op, const lmap_t *pos)
{
    sx_t lv, rv;
    int cond;

    assert(l);
    assert(r);
    assert(pos);

    lv = l->u.s;
    rv = r->u.s;

    if (rv == 0) {
        if (!silent)
            err_dmpos(pos, ERR_EXPR_DIVBYZERO, lmap_range(r->spos, r->epos), NULL);
        return 0;
    }

    cond = !(lv == SMIN && rv == -1);
    if (!cond && !silent)
        err_dmpos(pos, ERR_PP_OVFCONST, lmap_range(l->spos, l->epos),
                  lmap_range(r->spos, r->epos), NULL);

    if (op == '/')
        return (lv == SMIN && rv == -1)? SMIN: lv / rv;
    else {
        assert(op == '%');
        return (lv == SMIN && rv == -1)? 0: lv % rv;
    }
}


/*
 *  converts the type of an expression result to unsigned type;
 *  note that the value argument may be modified;
 *  ASSUMPTION: signed integers are compatible with unsigned ones on both
 */
static expr_t *castu(expr_t *r, const lmap_t *pos)
{
    assert(r);
    assert(pos);

    if (r->type == EXPR_TS) {
        if (r->u.s < 0) {
            if (!silent)
                err_dmpos(pos, ERR_PP_NEGTOUNSIGN, lmap_range(r->spos, r->epos), NULL);
            r->u.u = CROPU(r->u.s);
        }
        r->type = EXPR_TU;
    }

    return r;
}


/*
 *  generates a new value for signed results
 */
static expr_t *newrs(sx_t v, const lmap_t *spos, const lmap_t *epos)
{
    expr_t *r = ARENA_ALLOC(strg_line, sizeof(*r));
    r->type = EXPR_TS;
    r->u.s = CROPS(v);
    r->posf = 0;
    r->spos = spos;
    r->epos = epos;

    return r;
}


/*
 *  generates a new value for unsigned results
 */
static expr_t *newru(ux_t v, const lmap_t *spos, const lmap_t *epos)
{
    expr_t *r = ARENA_ALLOC(strg_line, sizeof(*r));
    r->type = EXPR_TU;
    r->u.u = CROPU(v);
    r->posf = 0;
    r->spos = spos;
    r->epos = epos;

    return r;
}


/*
 *  recognizes an integer constant;
 *  ASSUMPTION: sx_t on the host can represent all unsigned integers on the target
 */
static expr_t *icon(lex_t *t, const char *cs)
{
    ux_t n;
    int ovf, d, ty;
    const char *s = cs, *hex = "0123456789abcdef";

    assert(t);
    assert(cs);

    ty = EXPR_TS;
    n = ovf = 0;
    if (cs[0] == '0' && (cs[1] == 'x' || cs[1] == 'X') &&
        isxdigit(((unsigned char *)cs)[2])) {    /* 0x[0-9] */
        cs++;    /* skips 0 */
        while (isxdigit(*(unsigned char *)++cs)) {
            d = strchr(hex, tolower(*(unsigned char *)cs)) - hex;
            if (n & ~(UMAX >> 4))
                ovf = 1;
            else
                n = (n << 4) + d;
        }
    } else {    /* 0 or other digits */
        if (*cs == '0')    /* octal */
            while (isdigit(*(unsigned char *)cs)) {
                d = *cs++ - '0';
                if (*cs == '8' || *cs == '9')
                    err_dpos(lmap_spell(t, s, cs, cs+1), ERR_CONST_ILLOCTESC);
                if (n & ~(UMAX >> 3))
                    ovf = 1;
                else
                    n = (n << 3) + d;
            }
        else    /* decimal */
            while (isdigit(*(unsigned char *)cs)) {
                d = *cs++ - '0';
                if (n > (UMAX - d) / 10)
                    ovf = 1;
                else
                    n = n * 10 + d;
            }
    }

    if (((cs[0] == 'u' || cs[0] == 'U') && (cs[1] == 'l' || cs[1] == 'L')) ||
        ((cs[0] == 'l' || cs[0] == 'L') && (cs[1] == 'u' || cs[1] == 'U'))) {
        ty = EXPR_TU;
        cs += 2;
    } else if (*cs == 'u' || *cs == 'U') {
        ty = EXPR_TU;
        cs++;
    } else if (*cs == 'l' || *cs == 'L')
        cs++;
    else if (*cs == '.' || *cs == 'e') {
        err_dpos(t->pos, ERR_PP_ILLOP, "floating-point constant");
        EXCEPT_RAISE(invexpr);
        /* code below never runs */
    }

    if (*cs != '\0') {
        const char *e;
        for (e = cs; *e; e++)
            continue;
        err_dpos(lmap_spell(t, s, cs, e), ERR_CONST_PPNUMSFX, cs, LEX_ICON);
        EXCEPT_RAISE(invexpr);
        /* code below never runs */
    } else if (ovf)
        err_dpos(t->pos, ERR_PP_OVFCONST);

    return (n > SMAX || ty == EXPR_TU)? newru(n, t->pos, t->pos): newrs(n, t->pos, t->pos);
}


/*
 *  parses a primary expression;
 *  ASSUMPTION: signed integers are compatible with unsigned ones on the host
 */
static expr_t *prim(lex_t **pt)
{
    expr_t *r;
    const char *cs;

    assert(pt);
    assert(*pt);

    cs = LEX_SPELL(*pt);
    switch((*pt)->id) {
        case LEX_ID:
            if ((*pt)->id == LEX_ID && strcmp(cs, "defined") == 0) {
                int paren = 0;
                const lmap_t *dpos,    /* defined */
                             *ppos,    /* ( or defined */
                             *ipos;    /* id */

                assert(!pushback);
                ppos = dpos = (*pt)->pos;
                NEXTSP(*pt);    /* consumes defined */
                if ((*pt)->id == '(') {
                    ppos = (*pt)->pos;
                    NEXTSP(*pt);    /* consumes ( */
                    paren = 1;
                }
                if ((*pt)->id != LEX_ID) {
                    err_dmpos(lmap_after(ppos), ERR_PP_NODEFID, dpos, NULL);
                    EXCEPT_RAISE(invexpr);
                    /* code below never runs */
                } else {
                    cs = LEX_SPELL(*pt);
                    if (paren) {
                        ipos = (*pt)->pos;
                        NEXTSP(*pt);    /* consumes id */
                        if ((*pt)->id != ')') {
                            err_dmpos(lmap_after(ipos), ERR_PP_NODEFRPAREN, dpos, NULL) &&
                                err_dpos(ppos, ERR_PARSE_TOMATCH, "(");
                            EXCEPT_RAISE(invexpr);
                            /* code below never runs */
                        }
                    }
                    r = newrs(mcr_redef(cs), dpos, (*pt)->pos);
                }
            } else {
                err_dpos((*pt)->pos, ERR_PP_EXPRUNDEFID, cs);
                r = newrs(0, (*pt)->pos, (*pt)->pos);
            }
            break;
        case LEX_CCON:
            {
                int w = 0;
                ux_t c = clx_ccon(*pt, &w);
                r = ((w && main_opt()->wchart == 1) || (!w && main_opt()->uchar))?
                        newru(c, (*pt)->pos, (*pt)->pos): newrs(c, (*pt)->pos, (*pt)->pos);
            }
            break;
        case LEX_PPNUM:
            r = icon(*pt, cs);
            break;
        case LEX_SCON:
            err_dpos((*pt)->pos, ERR_PP_ILLOP, "string literal");
            EXCEPT_RAISE(invexpr);
            /* code below never runs */
            break;
        default:
            if ((*pt)->id == LEX_NEWLINE)
                err_dpos((*pt)->pos, ERR_PP_EXPRERR, "operand", name(*pt));
            else
                err_dpos((*pt)->pos, ERR_PP_ILLEXPR);
            EXCEPT_RAISE(invexpr);
            /* code below never runs */
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
            case '[':
            case '(':
            case '.':
            case LEX_DEREF:    /* -> */
            case LEX_INCR:     /* ++ */
            case LEX_DECR:     /* -- */
                err_dpos((*pt)->pos, ERR_PP_ILLOP, LEX_SPELL(*pt));
                EXCEPT_RAISE(invexpr);
                /* code below never runs */
            default:
                return l;
        }

    /* assert(!"invalid control flow -- should never reach here");
       return l; */
}


/*
 *  parses a unary expression;
 *  ASSUMPTION: overflow from negation is benign;
 *  ASSUMPTION: the target has no signed zero
 */
static expr_t *unary(lex_t **pt)
{
    expr_t *l;
    const lmap_t *spos;

    assert(pt);
    assert(*pt);

    switch((*pt)->id) {
        case LEX_INCR:    /* ++ */
        case LEX_DECR:    /* -- */
        case '&':
        case '*':
            err_dpos((*pt)->pos, ERR_PP_ILLOP, LEX_SPELL(*pt));
            EXCEPT_RAISE(invexpr);
            /* code below never runs */
        case '+':
            spos = (*pt)->pos;
            *pt = nextnsp();
            l = unary(pt);
            l->spos = spos;
            break;
        case '-':
            spos = (*pt)->pos;
            *pt = nextnsp();
            l = unary(pt);
            if (l->type == EXPR_TS)
                l->u.s = CROPS(-l->u.s);
            else {
                if (!silent)
                    err_dmpos(spos, ERR_EXPR_NEGUNSIGNED, lmap_range(l->spos, l->epos), NULL);
                l->u.u = CROPU(-l->u.u);
            }
            l->spos = spos;
            break;
        case '~':
            spos = (*pt)->pos;
            *pt = nextnsp();
            l = unary(pt);
            if (l->type == EXPR_TS)
                l->u.s = CROPS(~l->u.s);
            else
                l->u.u = CROPU(~l->u.u);
            l->spos = spos;
            break;
        case '!':
            spos = (*pt)->pos;
            *pt = nextnsp();
            l = unary(pt);
            l->u.u = (l->u.u == 0);
            l->type = EXPR_TS;
            l->spos = spos;
            break;
        case '(':
            spos = (*pt)->pos;
            if (level++ == TL_PARENE_STD)
                err_dpos(spos, ERR_PARSE_MANYPE) &&
                    err_dpos(spos, ERR_PARSE_MANYPESTD, (long)TL_PARENE_STD);
            *pt = nextnsp();
            l = postfix(pt, expr(pt, ')', spos));
            l->spos = spos;
            level--;
            break;
        default:
            l = postfix(pt, prim(pt));
            assert(l->posf == 0);
            break;
    }

    l->posf = 0;
    return l;
}


/*
 *  evaluates binary operators for unsigned results;
 *  ASSUMPTION: over-shift or negative shift is benign
 */
static expr_t *evalbinu(int op, expr_t *l, expr_t *r, const lmap_t *pos)
{
    ux_t lv, rv;

    assert(l);
    assert(r);
    assert(pos);

    switch(op) {
        /* result has type from usual arithmetic conversion */
        case '&':
        case '^':
        case '|':
            if (l->posf == 2)
                err_dpos(lmap_range(l->spos, l->epos), ERR_EXPR_NEEDPAREN);
            if (r->posf == 2)
                err_dpos(lmap_range(r->spos, r->epos), ERR_EXPR_NEEDPAREN);
            /* no break */
        case '*':
        case '/':
        case '%':
        case '+':
        case '-':
            lv = castu(l, pos)->u.u;    /* l's type modified */
            rv = castu(r, pos)->u.u;
            switch(op) {
                case '*':
                    l->u.s = CROPU(lv * rv);
                    break;
                case '/':
                case '%':
                    if (rv == 0) {
                        if (!silent)
                            err_dmpos(pos, ERR_EXPR_DIVBYZERO, lmap_range(r->spos, r->epos),
                                      NULL);
                        l->u.u = 0;
                    } else
                        l->u.u = CROPU((op == '/')? lv / rv: lv % rv);
                    break;
                case '+':
                    l->u.u = CROPU(lv + rv);
                    break;
                case '-':
                    l->u.s = CROPU(lv - rv);
                    break;
                case '&':
                    l->u.u = lv & rv;
                    break;
                case '^':
                    l->u.u = lv ^ rv;
                    break;
                case '|':
                    l->u.u = lv | rv;
                    break;
                default:
                    assert(!"invalid binary operator -- should never reach here");
                    break;
            }
            break;

        /* result has l's type */
        case LEX_LSHFT:    /* << */
            if (r->type == EXPR_TS) {
                if ((r->u.s < 0 || r->u.s >= PPINT_BYTE*TG_CHAR_BIT) && !silent)
                    err_dmpos(pos, ERR_EXPR_OVERSHIFTS, lmap_range(r->spos, r->epos), NULL,
                              (long)r->u.s);
                l->u.u = CROPU(l->u.u << r->u.s);
            } else {
                if (r->u.u >= PPINT_BYTE*TG_CHAR_BIT && !silent)
                    err_dmpos(pos, ERR_EXPR_OVERSHIFTU, lmap_range(r->spos, r->epos), NULL,
                              (unsigned long)r->u.u);
                l->u.u = CROPU(l->u.u << r->u.u);
            }
            l->spos = l->spos;
            l->epos = r->epos;
            break;
        case LEX_RSHFT:    /* >> */
            if (r->type == EXPR_TS) {
                if ((r->u.s < 0 || r->u.s >= PPINT_BYTE*TG_CHAR_BIT) && !silent)
                    err_dmpos(pos, ERR_EXPR_OVERSHIFTS, lmap_range(r->spos, r->epos), NULL,
                              (long)r->u.s);
                l->u.u = CROPU(l->u.u >> r->u.s);
            } else {
                if (r->u.u >= PPINT_BYTE*TG_CHAR_BIT && !silent)
                    err_dmpos(pos, ERR_EXPR_OVERSHIFTU, lmap_range(r->spos, r->epos), NULL,
                              (unsigned long)r->u.u);
                l->u.u = CROPU(l->u.u >> r->u.u);
            }
            l->spos = l->spos;
            l->epos = r->epos;
            break;

        /* usual arithmetic conversion; result has signed type */
        case '<':
        case '>':
        case LEX_LEQ:     /* <= */
        case LEX_GEQ:     /* >= */
        case LEX_EQEQ:    /* == */
        case LEX_NEQ:     /* != */
            lv = castu(l, pos)->u.u;    /* l's type modified */
            rv = castu(r, pos)->u.u;
            switch(op) {
                case '<':
                    l->u.u = (lv < rv);
                    break;
                case '>':
                    l->u.u = (lv > rv);
                    break;
                case LEX_LEQ:    /* <= */
                    l->u.u = (lv <= rv);
                    break;
                case LEX_GEQ:    /* >= */
                    l->u.u = (lv >= rv);
                    break;
                case LEX_EQEQ:    /* == */
                    l->u.u = (lv == rv);
                    break;
                case LEX_NEQ:    /* != */
                    l->u.u = (lv != rv);
                    break;
                default:
                    assert(!"invalid binary operator -- should never reach here");
                    break;
            }
            l->type = EXPR_TS;
            l->posf = 2;
            l->spos = l->spos;
            l->epos = r->epos;
            return l;

        default:
            assert(!"invalid binary operator -- should never reach here");
            break;
    }

    l->posf = 0;
    l->spos = l->spos;
    l->epos = r->epos;
    return l;
}


/*
 *  evaluates binary operators for signed results;
 *  no conversion occurs, which simplifies code;
 *  ASSUMPTION: bitwise operators do the same as on the target;
 *  ASSUMPTION: over-shift or negative shift is benign
 */
static expr_t *evalbins(int op, expr_t *l, expr_t *r, const lmap_t *pos)
{
    assert(l);
    assert(r);
    assert(pos);

    switch(op) {
        case '*':
            l->u.s = mul(l, r, pos);
            break;
        case '/':
        case '%':
            l->u.s = mdiv(l, r, op, pos);
            break;
        case '+':
            l->u.s = add(l, r, pos);
            break;
        case '-':
            l->u.s = sub(l, r, pos);
            break;
        case LEX_LSHFT:    /* << */
            if (l->u.s < 0 && !silent)
                err_dmpos(pos, ERR_EXPR_LSHIFTNEG, lmap_range(l->spos, l->epos), NULL);
            if (r->type == EXPR_TS) {
                if ((r->u.s < 0 || r->u.s >= PPINT_BYTE*TG_CHAR_BIT ||
                     (l->u.s && r->u.s >= PPINT_BYTE*TG_CHAR_BIT-1)) && !silent)
                    err_dmpos(pos, ERR_EXPR_OVERSHIFTS, lmap_range(r->spos, r->epos), NULL,
                              (long)r->u.s);
                else if (l->u.s >= 0)
                    mul(l, newrs(((sx_t)1) << r->u.s, r->spos, r->epos), pos);
                l->u.s = CROPS(l->u.s << r->u.s);
            } else {
                if ((r->u.u >= PPINT_BYTE*TG_CHAR_BIT ||
                     (l->u.s && r->u.u >= PPINT_BYTE*TG_CHAR_BIT-1)) && !silent)
                    err_dmpos(pos, ERR_EXPR_OVERSHIFTU, lmap_range(r->spos, r->epos), NULL,
                              (unsigned long)r->u.u);
                else if (l->u.s >= 0)
                    mul(l, newrs(((sx_t)1) << r->u.u, r->spos, r->epos), pos);
                l->u.s = CROPS(l->u.s << r->u.u);
            }
            break;
        case LEX_RSHFT:    /* >> */
            {
                sx_t n;

                if (l->u.s < 0 && !silent)
                    err_dmpos(pos, ERR_EXPR_RSHIFTNEG, lmap_range(l->spos, l->epos), NULL);
                if (main_opt()->logicshift)
                    return evalbinu(op, l, r, pos);
                if (r->type == EXPR_TS) {
                    if ((r->u.s < 0 || r->u.s >= PPINT_BYTE*TG_CHAR_BIT) && !silent)
                        err_dmpos(pos, ERR_EXPR_OVERSHIFTS, lmap_range(r->spos, r->epos), NULL,
                                  (long)r->u.s);
                    n = CROPS(l->u.s >> r->u.s);
                    if (r->u.s >= 0 && l->u.s < 0 && n >= 0)
                        n |= ~(~0UL >> r->u.s);
                } else {
                    if (r->u.u >= PPINT_BYTE*TG_CHAR_BIT && !silent)
                        err_dmpos(pos, ERR_EXPR_OVERSHIFTU, lmap_range(r->spos, r->epos), NULL,
                                  (unsigned long)r->u.u);
                    n = CROPS(l->u.s >> r->u.u);
                    if (l->u.s < 0 && n >= 0)
                        n |= ~(~0UL >> r->u.u);
                }
                /* cannot assert(l->u.s >= 0 || n < 0) */
                l->u.s = n;
            }
            break;
        case '<':
        case '>':
        case LEX_LEQ:     /* <= */
        case LEX_GEQ:     /* >= */
        case LEX_EQEQ:    /* == */
        case LEX_NEQ:     /* != */
            switch(op) {
                case '<':
                    l->u.s = (l->u.s < r->u.s);
                    break;
                case '>':
                    l->u.s = (l->u.s > r->u.s);
                    break;
                case LEX_LEQ:    /* <= */
                    l->u.s = (l->u.s <= r->u.s);
                    break;
                case LEX_GEQ:    /* >= */
                    l->u.s = (l->u.s >= r->u.s);
                    break;
                case LEX_EQEQ:    /* == */
                    l->u.s = (l->u.s == r->u.s);
                    break;
                case LEX_NEQ:    /* != */
                    l->u.s = (l->u.s != r->u.s);
                    break;
                default:
                    assert(!"invalid binary operator -- should never reach here");
                    break;
            }
            l->posf = 2;
            l->spos = l->spos;
            l->epos = r->epos;
            return l;
        case '&':
        case '^':
        case '|':
            if (l->posf == 2)
                err_dpos(lmap_range(l->spos, l->epos), ERR_EXPR_NEEDPAREN);
            if (r->posf == 2)
                err_dpos(lmap_range(r->spos, r->epos), ERR_EXPR_NEEDPAREN);
            switch(op) {
                case '&':
                    l->u.s = CROPS(l->u.s & r->u.s);
                    break;
                case '^':
                    l->u.s = CROPS(l->u.s ^ r->u.s);
                    break;
                case '|':
                    l->u.s = CROPS(l->u.s | r->u.s);
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

    l->posf = 0;
    l->spos = l->spos;
    l->epos = r->epos;
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

    if (op == LEX_LSHFT || op == LEX_RSHFT)
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
            const lmap_t *pos = (*pt)->pos;

            t = preptok();
            pushback = t;
            if (t->id == '=')
                break;
            *pt = nextnsp();
            r = bin(pt, k1+1);    /* no side effect; can evaluate both */
            l = eval[type(tid, l, r)](tid, l, r, pos);
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
    expr_t *l, *r;
    int os = silent;

    assert(pt);
    assert(*pt);

    l = bin(pt, 6);
    if ((*pt)->id == LEX_ANDAND) {
        if (!l->u.u)
            silent++;
        do {
            l->posf = 1;
            *pt = nextnsp();
            r = bin(pt, 6);
            if (!r->u.u)
                silent++;
        } while((*pt)->id == LEX_ANDAND);
        l->type = EXPR_TS;
        if (silent != os) {
            silent = os;
            l->u.u = 0;
        } else
            l->u.u = 1;
        l->spos = l->spos;
        l->epos = r->epos;
    }

    return l;
}


/*
 *  parses a logical-OR expression;
 *  separated from bin() to implement short-circuit
 */
static expr_t *or(lex_t **pt)
{
    expr_t *l, *r;
    int os = silent;

    assert(pt);
    assert(*pt);

    l = and(pt);
    if ((*pt)->id == LEX_OROR) {
        if (l->posf == 1)
            err_dpos(lmap_range(l->spos, l->epos), ERR_EXPR_NEEDPAREN);
        if (l->u.u)
            silent++;
        do {
            *pt = nextnsp();
            r = and(pt);
            if (r->posf == 1)
                err_dpos(lmap_range(r->spos, r->epos), ERR_EXPR_NEEDPAREN);
            if (r->u.u)
                silent++;
        } while ((*pt)->id == LEX_OROR);
        l->type = EXPR_TS;
        if (silent != os) {
            silent = os;
            l->u.u = 1;
        } else
            l->u.u = 0;
        l->spos = l->spos;
        l->epos = r->epos;
    }

    l->posf = 0;
    return l;
}


/*
 *  parses a conditional expression
 */
static expr_t *cond(lex_t **pt)
{
    expr_t *c, *l, *r;
    const lmap_t *pos;

    assert(pt);
    assert(*pt);

    c = or(pt);
    if ((*pt)->id == '?') {
        pos = (*pt)->pos;
        *pt = nextnsp();
        /* no side effect, thus can evaluate both */
        if (c->u.u)
            l = expr(pt, ':', pos);
        else {
            silent++;
            l = expr(pt, ':', pos);
            silent--;
        }
        if (c->u.u) {
            silent++;
            r = cond(pt);
            silent--;
        } else
            r = cond(pt);
        if (type('?', l, r) == EXPR_TU) {
            l = castu(l, pos);
            r = castu(r, pos);
        }
        l = (c->u.u)? l: r;
        l->posf = 0;
        return l;
    }

    return c;
}


/*
 *  parses an assignment expression
 */
static expr_t *asgn(lex_t **pt)
{
    expr_t *r;

    assert(pt);
    assert(*pt);

    r = cond(pt);
    if (prec[(*pt)->id] == 2) {
        err_dpos((*pt)->pos, ERR_PP_ILLOP, LEX_SPELL(*pt));
        EXCEPT_RAISE(invexpr);
        /* code below never runs */
    }

    return r;
}


/*
 *  parses an expression
 */
static expr_t *expr(lex_t **pt, int tid, const lmap_t *pos)
{
    expr_t *r;
    char s[4] = "` '";

    assert(pt);
    assert(*pt);

    r = asgn(pt);
    if ((*pt)->id == ',') {
        if (main_opt()->std == 1 || !silent)
            err_dpos((*pt)->pos, ERR_PP_ILLOPW, ",");
        /* accepts , */
        do {
            *pt = nextnsp();
            if ((*pt)->id == LEX_NEWLINE)
                break;
            r = asgn(pt);
        } while((*pt)->id == ',');
    }
    if (tid) {
        if ((*pt)->id == tid) {
            if (tid == ')')
                r->epos = (*pt)->pos;
            *pt = nextnsp();
        } else {
            s[1] = tid;
            err_dpos(lmap_after(r->epos), ERR_PP_EXPRERR, s, name(*pt)) &&
                err_dpos(pos, ERR_PARSE_TOMATCH, (tid == ')')? "(": "?");
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
    assert(sizeof(sx_t)*CHAR_BIT >= PPINT_BYTE*TG_CHAR_BIT);

    level = silent = 0;
    pushback = *pt;
    *pt = nextnsp();
    if ((*pt)->id == LEX_NEWLINE)
        err_dpos((*pt)->pos, ERR_PP_NOIFEXPR, k);
    else {
        EXCEPT_TRY
            r = expr(pt, 0, NULL);
            if ((*pt)->id == LEX_NEWLINE)
                EXCEPT_RETURN r;
            else {
                switch((*pt)->id) {
                    case ')':    /* closing paren */
                        err_dpos((*pt)->pos, ERR_PP_NOEXPRLPAREN, NULL);
                        break;
                    /* operands */
                    case LEX_ID:
                    case LEX_CCON:
                    case LEX_PPNUM:
                        err_dmpos(lmap_after(r->epos), ERR_PP_EXPRERR, (*pt)->pos, NULL,
                                  "operator", name(*pt));
                        break;
                    case LEX_SCON:
                        err_dpos((*pt)->pos, ERR_PP_ILLOP, "string literal");
                        break;
                    default:
                        err_dpos((*pt)->pos, ERR_PP_ILLEXPR);
                        break;
                }
            }
        EXCEPT_EXCEPT(invexpr)
            /* nothing to do */
        EXCEPT_END
    }

    return newrs(0, (*pt)->pos, (*pt)->pos);
}

/* end of expr.c */
