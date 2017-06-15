/*
 *  constant folding and optimization
 */

#include <stddef.h>        /* NULL */
#include <cbl/assert.h>    /* assert */

#include "clx.h"
#include "common.h"
#include "enode.h"
#include "err.h"
#include "expr.h"
#include "ir.h"
#include "lex.h"
#include "op.h"
#include "stmt.h"
#include "sym.h"
#include "tree.h"
#include "ty.h"
#include "simp.h"

/* additional wrappers for convenience */
#define xbau       xba             /* for foldbinnv() */
#define xbou       xbo
#define xbxu       xbx
#define xes        xe              /* for chkdivby0() and foldcmp() */
#define xeu        xe
#define xef(x, y)  ((x) == (y))
#define xed        xef
#define xeld       xef
#define xnef(x, y) ((x) != (y))    /* for foldcmp() */
#define xned       xnef
#define xneld      xnef
#define xnes       xne
#define xneu       xne
#define xgef(x, y) ((x) >= (y))
#define xged       xgef
#define xgeld      xgef            /* also for cvtov() */
#define xgf(x, y)  ((x) > (y))
#define xgd        xgf
#define xgld       xgf
#define xlef(x, y) ((x) <= (y))
#define xled       xlef            /* also for cvtov() */
#define xleld      xlef
#define xlf(x, y)  ((x) < (y))
#define xld        xlf
#define xlld       xlf

#define chkxad  chkxaf
#define chkxald chkxaf
#define chkxsd  chkxsf
#define chkxsld chkxsf
#define chkxmd  chkxmf
#define chkxmld chkxmf
#define chkxdd  chkxdf
#define chkxdld chkxdf
#define chkxrs  chkxds

#define doxaf(l, r) ((l) + (r))
#define doxad       doxaf
#define doxald      doxaf
#define doxsf(l, r) ((l) - (r))
#define doxsd       doxsf
#define doxsld      doxsf
#define doxmf(l, r) ((l) * (r))
#define doxmd       doxmf
#define doxmld      doxmf
#define doxdf(l, r) ((l) / (r))
#define doxdd       doxdf
#define doxdld      doxdf
#define doxas(l, r) (xas((l), (r)))
#define doxss(l, r) (xss((l), (r)))
#define doxms(l, r) (domuls((l), (r)))
#define doxds(l, r) (dodivs((l), (r), 0))
#define doxrs(l, r) (dodivs((l), (r), 1))

#define SYM_CROPF
#define SYM_CROPD
#define SYM_CROPX

/* folds constants for binary operations with no overflow */
#define foldbinnv(OP, CROP)                                                         \
    if (l->op == OP_CNST+sfx && r->op == OP_CNST+sfx) {                             \
        return tree_uconst(SYM_CROP##CROP(OP##u(l->u.v.u, r->u.v.u)), ty, tpos);    \
    }

/* folds constants for binary operations with overflow */
#define foldbinov(VAR, CROP, FUNC, MIN, MAX, needconst)                        \
    if (l->op == OP_CNST+sfx && r->op == OP_CNST+sfx &&                        \
        chk##FUNC##VAR(l, r, #VAR, MIN, MAX, needconst, tpos)) {               \
        p = tree_new(OP_CNST+sfx, ty, NULL, NULL, tpos);                       \
        p->u.v.VAR = SYM_CROP##CROP(do##FUNC##VAR(l->u.v.VAR, r->u.v.VAR));    \
        return p;                                                              \
    }

/* checks if dividend is 0 */
#define chkdivby0(VAR, Z)                                    \
    if (r->op == OP_CNST+sfx && xe##VAR(r->u.v.VAR, Z)) {    \
        err_dtpos(tpos, NULL, r, ERR_EXPR_DIVBYZERO);        \
        break;                                               \
    }

/* removes identities */
#define identity(X, Y, TYPE, VAR, VAL)                                     \
    if (op_optype(X->op) == OP_CNST+OP_##TYPE && xe(X->u.v.VAR, VAL)) {    \
        return TREE_RVAL(Y, tpos);                                         \
    }

/* removes operations with no effect */
#define noeffect(X, Y, VAR, VAL, EXPR, RVAL)                        \
    if (X->op == OP_CNST+sfx && xe(X->u.v.VAR, VAL)) {              \
        err_dtpos(tpos, X, NULL, ERR_EXPR_NOEFFECT, (long)RVAL);    \
        return tree_right(tree_root(Y), EXPR, X->type, tpos);       \
    }

/* makes right child of tree have constant if any */
#define commute(L, R)                                                      \
    if (op_generic(R->op) == OP_CNST && op_generic(L->op) != OP_CNST) {    \
        tree_t *t = L; L = R; R = t;                                       \
    }

/* folds constants for comparison operations */
#define foldcmp(VAR, OP)                                                               \
    if (l->op == OP_CNST+sfx && r->op == OP_CNST+sfx) {                                \
        return tree_sconst(xis(OP##VAR(l->u.v.VAR, r->u.v.VAR)), ty_inttype, tpos);    \
    }

/* folds constants for unary operations */
#define folduni(EXP) if (l->op == OP_CNST+sfx) { return EXP; }

/* folds constants for logical operations */
#define foldlog(C)                                                                 \
    if (l->op == OP_CNST+sfx) {                                                    \
        int npce = l->f.npce;                                                      \
        if (C) {                                                                   \
            r->f.npce &= (TREE_FACE|TREE_FICE);    /* masks COMMA|ADDR */          \
            if (main_opt()->std == 1)                                              \
                npce |= tree_chkinit(r);    /* ADDR */                             \
            npce |= ((r->op == OP_CNST+sfx)? r->f.npce: (TREE_FACE|TREE_FICE));    \
            p = l;                                                                 \
        } else                                                                     \
            p = r;                                                                 \
        p->f.npce |= npce;                                                         \
        return tree_retype(p, NULL, tpos);                                         \
    }

/* folds constants for shift operations with no overflow */
#define foldshnv(OP, CROP)                                                                   \
    if (l->op == OP_CNST+sfx && op_optype(r->op) == OP_CNST+OP_I && xges(r->u.v.s, xO) &&    \
        xls(r->u.v.s, xis(TG_CHAR_BIT*l->type->size))) {                                     \
        return tree_uconst(SYM_CROP##CROP(OP(SYM_CROP##CROP(l->u.v.u), xns(r->u.v.s))),      \
                           ty, tpos);                                                        \
    }

/* folds constants for left-shift operation with overflow */
#define foldlshov()                                                                          \
    if (l->op == OP_CNST+sfx && op_optype(r->op) == OP_CNST+OP_I &&                          \
        xges(l->u.v.s, xO) && xges(r->u.v.s, xO) &&                                          \
        xls(r->u.v.s, xis(TG_CHAR_BIT*l->type->size)) &&                                     \
        chkxms(l, tree_sconst(xsl(xI, xns(r->u.v.s)), ty, r->orgn->pos), "s",                \
               oty->u.sym->u.lim.min.s, oty->u.sym->u.lim.max.s, simp_needconst, tpos)) {    \
        return tree_sconst(xsl(l->u.v.s, xns(r->u.v.s)), ty, tpos);                          \
    }

/* folds constans for right-shift operation with overflow;
   ASSUMPTION: logical right shift handled elsewhere */
#define foldrshov()                                                                          \
    if (l->op == OP_CNST+sfx && op_optype(r->op) == OP_CNST+OP_I && xges(r->u.v.s, xO) &&    \
        xls(r->u.v.s, xis(TG_CHAR_BIT*l->type->size))) {                                     \
        return tree_sconst(xsra(l->u.v.s, xns(r->u.v.s)), ty, tpos);                         \
    }

/* warns left shift of negative values */
#define warnneglsh()                                            \
    if (op_generic(l->op) == OP_CNST && xls(l->u.v.s, xO)) {    \
        err_dtpos(tpos, l, NULL, ERR_EXPR_LSHIFTNEG);           \
    }

/* warns right shift of negative values */
#define warnnegrsh()                                            \
    if (op_generic(l->op) == OP_CNST && xls(l->u.v.s, xO)) {    \
        err_dtpos(tpos, l, NULL, ERR_EXPR_RSHIFTNEG);           \
    }

/* warns overshifting */
#define warnovsh(sign)                                                             \
    if (op_optype(r->op) == OP_CNST+OP_I &&                                        \
        (xls(r->u.v.s, xO) || xges(r->u.v.s, xis(TG_CHAR_BIT*l->type->size)) ||    \
         ((sign) && op_generic(l->op) == OP_CNST && xne(l->u.v.s, xO) &&           \
          xges(r->u.v.s, xis(TG_CHAR_BIT*l->type->size-1))))) {                    \
        err_dtpos(tpos, NULL, r, ERR_EXPR_OVERSHIFTS, r->u.v.s);                   \
        break;                                                                     \
    }

/* warns overshifting for converted but unfolded operands */
#define warnovshcv()                                                            \
    if (OP_ISCV(r->op) && op_generic(r->kid[0]->op) == OP_CNST) {               \
        if (OP_ISSINT(r->kid[0]->op))                                           \
            err_dtpos(tpos, NULL, r, ERR_EXPR_OVERSHIFTS, r->kid[0]->u.v.s);    \
        else if (OP_ISUINT(r->kid[0]->op))                                      \
            err_dtpos(tpos, NULL, r, ERR_EXPR_OVERSHIFTU, r->kid[0]->u.v.u);    \
        else                                                                    \
            err_dtpos(tpos, NULL, r, ERR_EXPR_OVERSHIFT);                       \
        break;                                                                  \
    }

/* folds constants for ADD+P operation;
   ASSUMPTION: pointer addition on the target can be emulated by integral addition */
#define foldaddp(L, R, RTYPE, VAR)                                           \
    if (L->op == OP_CNST+sfx && op_optype(R->op) == OP_CNST+OP_##RTYPE) {    \
        p = tree_new(OP_CNST+sfx, ty, NULL, NULL, tpos);                     \
        p->u.v.p = xas(L->u.v.p, R->u.v.VAR);                                \
        return p;                                                            \
    }

/* distributes constants to fold */
#define distribute(OP)                                                                      \
    if (l->op == OP_CNST+sfx && r->op == OP_##OP+sfx &&                                     \
        (r->kid[0]->op == OP_CNST+sfx || r->kid[1]->op == OP_CNST+sfx)) {                   \
        return simp_tree(OP_##OP, ty, simp_tree(OP_MUL, oty, l, r->kid[0], tpos),           \
                                      simp_tree(OP_MUL, oty, l, r->kid[1], tpos), tpos);    \
    }

/* converts signed operation to shift;
   ASSUMPTION: 2sC for signed integers assumed */
#define stoshift(X, Y, OP)                                                                \
    if (X->op == OP_CNST+sfx && xgs(X->u.v.s, xO) && (n = ispow2(X->u.v.s)) != 0) {       \
        return simp_tree(OP_##OP, ty, Y, tree_sconst(xis(n), ty_inttype, tpos), tpos);    \
    }

/* converts unsigned operation to shift */
#define utoshift(X, Y, OP)                                                                \
    if (X->op == OP_CNST+sfx && (n = ispow2(X->u.v.u)) != 0) {                            \
        return simp_tree(OP_##OP, ty, Y, tree_sconst(xis(n), ty_inttype, tpos), tpos);    \
    }

/* gets rid of unnecessary bit extraction */
#define zerofield(OP, VAR)                                                                        \
    {                                                                                             \
        tree_t *q = l;                                                                            \
        if (OP_ISCV(q->op))                                                                       \
            q = l->kid[0];                                                                        \
        if (q->op == OP_RIGHT && !q->kid[0])                                                      \
            q = q->kid[1];                                                                        \
        if (q->op == OP_FIELD && r->op == OP_CNST+sfx && xe(r->u.v.VAR, xO)) {                    \
            assert(TY_ISINTEGER(q->type));                                                        \
            return tree_cmp(OP_##OP,                                                              \
                       tree_bit(OP_BAND, q->kid[0],                                               \
                           tree_uconst(xsl(SYM_FLDMASK(q->u.field), SYM_FLDRIGHT(q->u.field)),    \
                                       ty_unsignedtype, tpos), NULL, tpos),                       \
                       tree_uconst(xO, ty_unsignedtype, tpos), NULL, tpos);                       \
        }                                                                                         \
    }

/* folds (in)equality comparison of symbol to zero */
#define symcmpz(V)                                                                   \
    if (op_optype(l->op) == OP_CVP+OP_U && OP_ISADDR(l->kid[0]->op) &&               \
        op_generic(r->op) == OP_CNST && xe(r->u.v.u, xO)) {                          \
        if (!l->kid[0]->u.sym->f.outofline) {                                        \
            p = tree_untype(simp_basetree(NULL, l->kid[0]));                         \
            err_dtpos(tpos, p, NULL, ERR_EXPR_SYMBOLTRUE, p->u.sym, " a symbol");    \
        }                                                                            \
        p = tree_sconst(xis(V), ty_inttype, tpos);                                   \
        p->f.npce |= (TREE_FADDR|TREE_FACE|TREE_FICE);                               \
        return p;                                                                    \
    }

/* removes unsigned comparisons that always result in constants */
#define geu(L, R, V)                                                             \
    if (R->op == OP_CNST+sfx && xe(R->u.v.u, xO)) {                              \
        err_dtpos(tpos, R, NULL, ERR_EXPR_UNSIGNEDCMP, (V)? "true": "false");    \
        return tree_right(tree_root(L),                                          \
                          tree_sconst(xis(V), ty_inttype, tpos),                 \
                          ty_inttype, tpos);                                     \
    }

/* removes operations that are meaningless when applied twice */
#define idempotent(OP)                       \
    if (l->op == OP) {                       \
        return TREE_RVAL(l->kid[0], tpos);   \
    }

/* changes unsigned relational comparisons to equality comparisons */
#define utoeq(C, OP)                                   \
    if (C->op == OP_CNST+sfx && xe(C->u.v.u, xO)) {    \
        return tree_cmp(OP_##OP, l, r, ty, tpos);      \
    }


/* removes conversion between similar types of same size */
#define samesize()                           \
    if (fty->size == tty->size) {            \
        return tree_retype(l, tty, NULL);    \
    }

/* converts constant whose result type has no overflow */
#define cvtnv(EXPR)                                                  \
    if (op_generic(l->op) == OP_CNST) {                              \
        p = tree_new(OP_CNST+sfx, tty, NULL, NULL, l->orgn->pos);    \
        EXPR;                                                        \
        return p;                                                    \
    }

/* converts constant whose result type has overflow */
#define cvtov(VAR, MIN, MAX, EXPR)                                                           \
    if (op_generic(l->op) == OP_CNST) {                                                      \
        /* diagnostics issued in enode_cast() if necessary */                                \
        if (simp_needconst || (xge##VAR(l->u.v.VAR, MIN) && xle##VAR(l->u.v.VAR, MAX))) {    \
            p = tree_new(OP_CNST+sfx, tty, NULL, NULL, l->orgn->pos);                        \
            EXPR;                                                                            \
            return p;                                                                        \
        }                                                                                    \
    }

/* converts constant of unsigned type to signed */
#define cvtus(MAX, EXPR)                                                 \
    if (op_generic(l->op) == OP_CNST) {                                  \
        /* diagnostics issued in enode_cast() if necessary */            \
        if (simp_needconst || xleu(l->u.v.u, MAX)) {                     \
            p = tree_new(OP_CNST+sfx, tty, NULL, NULL, l->orgn->pos);    \
            EXPR;                                                        \
            return p;                                                    \
        }                                                                \
    }


int simp_needconst;    /* > 0 while constant is needed */


#define CHOOSEFPV()                                         \
    if (ty[0] == 'd') lv = l->u.v.d, rv = r->u.v.d;         \
    else if (ty[0] == 'f') lv = l->u.v.f, rv = r->u.v.f;    \
    else lv = l->u.v.ld, rv = r->u.v.ld;

/*
 *  checks if a signed integer expression overflows for addition
 */
static int chkxas(const tree_t *l, const tree_t *r, const char *ty, sx_t min, sx_t max,
                  int needconst, tree_pos_t *tpos)
{
    int cond;
    sx_t lv, rv;

    assert(l);
    assert(r);
    UNUSED(ty);
    assert(tpos);

    lv = l->u.v.s;
    rv = r->u.v.s;
    cond = (xe(lv, xO) || xe(rv, xO) ||
            (xls(lv, xO) && xgs(rv, xO)) ||
            (xgs(lv, xO) && xls(rv, xO)) ||
            (xls(lv, xO) && xls(rv, xO) && xges(lv, xss(min, rv))) ||
            (xgs(lv, xO) && xgs(rv, xO) && xles(lv, xss(max, rv))));

    if (!cond && needconst) {
        err_dtpos(tpos, l, r, ERR_EXPR_OVFCONST);
        cond = 1;
    }

    return cond;
}


/*
 *  checks if a floating-point expression overflows for addition;
 *  ASSUMPTION: fp addition on the host works in reasonable way;
 *  ASSUMPTION: rounding direction on the host is same as on the target
 */
static int chkxaf(const tree_t *l, const tree_t *r, const char *ty, long double min,
                  long double max, int needconst, tree_pos_t *tpos)
{
    int cond;
    long double lv, rv;

    assert(l);
    assert(r);
    assert(ty);
    assert(tpos);

    CHOOSEFPV();

    cond = (lv == 0 || rv == 0 ||
            (lv < 0 && rv > 0) ||
            (lv > 0 && rv < 0) ||
            (lv < 0 && rv < 0 && lv >= min-rv) ||
            (lv > 0 && rv > 0 && lv <= max-rv));

    if (!cond)
        err_dtpos(tpos, l, r, ERR_EXPR_OVFCONSTFP);
    if (needconst)
        cond = 1;

    return cond;
}


/*
 *  checks if a signed integer expression overflows for subtraction
 */
static int chkxss(const tree_t *l, const tree_t *r, const char *ty, sx_t min, sx_t max,
                  int needconst, tree_pos_t *tpos)
{
    int cond;
    sx_t lv, rv;

    assert(l);
    assert(r);
    UNUSED(ty);
    assert(tpos);

    lv = l->u.v.s;
    rv = r->u.v.s;
    cond = (xe(lv, xO) || xe(rv, xO) ||
            (xls(lv, xO) && xls(rv, xO)) ||
            (xgs(lv, xO) && xgs(rv, xO)) ||
            (xls(lv, xO) && xgs(rv, xO) && xges(lv, xas(min, rv))) ||
            (xgs(lv, xO) && xls(rv, xO) && xles(lv, xas(max, rv))));

    if (!cond && needconst) {
        err_dtpos(tpos, l, r, ERR_EXPR_OVFCONST);
        cond = 1;
    }

    return cond;
}


/*
 *  checks if a floating-point expression overflows for subtraction;
 *  ASSUMPTION: fp subtraction on the host works in reasonable way;
 *  ASSUMPTION: rounding direction on the host is same as on the target
 */
static int chkxsf(const tree_t *l, const tree_t *r, const char *ty, long double min,
                  long double max, int needconst, tree_pos_t *tpos)
{
    int cond;
    long double lv, rv;

    assert(l);
    assert(r);
    assert(ty);
    assert(tpos);

    CHOOSEFPV();
    rv = -rv;

    /* same as chkxaf() above */
    cond = (lv == 0 || rv == 0 ||
            (lv < 0 && rv > 0) ||
            (lv > 0 && rv < 0) ||
            (lv < 0 && rv < 0 && lv >= min-rv) ||
            (lv > 0 && rv > 0 && lv <= max-rv));

    if (!cond)
        err_dtpos(tpos, l, r, ERR_EXPR_OVFCONSTFP);
    if (needconst)
        cond = 1;

    return cond;
}


/*
 *  checks if a signed integer expression overflows for multipication;
 *  ASSUMPTION: 2sC for signed integers assumed
 */
static int chkxms(const tree_t *l, const tree_t *r, const char *ty, sx_t min, sx_t max,
                  int needconst, tree_pos_t *tpos)
{
    int cond;
    sx_t lv, rv;

    assert(l);
    assert(r);
    UNUSED(ty);
    assert(tpos);

    lv = l->u.v.s;
    rv = r->u.v.s;
    cond = (!(xe(lv, x_I) && xe(rv, min)) &&
            !(xe(lv, min) && xe(rv, x_I)) &&
            ((xe(lv, xO) || xe(rv, xO)) ||
             (xls(lv, xO) && xls(rv, xO) && xges(lv, xvs(max, rv))) ||
             (xls(lv, xO) && xgs(rv, xO) && xges(lv, xvs(min, rv))) ||
             (xgs(lv, xO) && xls(rv, xO) && (xe(rv, x_I) || xles(lv, xvs(min, rv)))) ||
             (xgs(lv, xO) && xgs(rv, xO) && xles(lv, xds(max, rv)))));

    if (!cond && needconst) {
        err_dtpos(tpos, l, r, ERR_EXPR_OVFCONST);
        cond = 1;
    }

    return cond;
}


/*
 *  checks if a floating-point expression overflows for multipication;
 *  ASSUMPTION: fp multiplication works in reasonable way;
 *  ASSUMPTION: rounding direction on the host is same as on the target
 */
static int chkxmf(const tree_t *l, const tree_t *r, const char *ty, long double min,
                  long double max, int needconst, tree_pos_t *tpos)
{
    int cond;
    long double lv, rv;

    assert(l);
    assert(r);
    assert(ty);
    assert(tpos);

    CHOOSEFPV();

    cond = ((lv >= -1 && lv <= 1) ||
            (rv >= -1 && rv <= 1) ||
            (lv == 0 || rv == 0) ||
            (lv < 0 && rv < 0 && lv >= max/rv) ||
            (lv < 0 && rv > 0 && lv >= min/rv) ||
            (lv > 0 && rv < 0 && lv <= min/rv) ||
            (lv > 0 && rv > 0 && lv <= max/rv));

    if (!cond)
        err_dtpos(tpos, l, r, ERR_EXPR_OVFCONSTFP);
    if (needconst)
        cond = 1;

    return cond;
}


/*
 *  performs integral multiplication to avoid an exception on the host
 */
static sx_t domuls(sx_t l, sx_t r)
{
    return ((xe(l, x_I) && xe(r, xmns)) || (xe(l, xmns) && xe(r, x_I)))? xmns: xms(l, r);
}


/*
 *  checks if a signed integer expression overflows for division;
 *  ASSUMPTION: 2sC for signed integers assumed
 */
static int chkxds(const tree_t *l, const tree_t *r, const char *ty, sx_t min, sx_t max,
                  int needconst, tree_pos_t *tpos)
{
    int cond;
    sx_t lv, rv;

    assert(l);
    assert(r);
    UNUSED(ty);
    UNUSED(max);
    assert(tpos);

    lv = l->u.v.s;
    rv = r->u.v.s;
    cond = !(xe(lv, min) && xe(rv, x_I));

    if (!cond && needconst) {
        err_dtpos(tpos, l, r, ERR_EXPR_OVFCONST);
        cond = 1;
    }

    return cond;
}


/*
 *  checks if a floating-point expression overflows for division;
 *  ASSUMPTION: fp division works in reasonable way;
 *  ASSUMPTION: rounding direction on the host is same as on the target
 */
static int chkxdf(const tree_t *l, const tree_t *r, const char *ty, long double min,
                  long double max, int needconst, tree_pos_t *tpos)
{
    int cond;
    long double lv, rv;

    assert(l);
    assert(r);
    assert(ty);
    UNUSED(min);
    assert(tpos);

    CHOOSEFPV();

    if (lv < 0)
        lv = -lv;
    if (rv < 0)
        rv = -rv;
    cond = !(rv < 1 && lv > max*rv);
    if (!cond)
        err_dtpos(tpos, l, r, ERR_EXPR_OVFCONSTFP);
    if (needconst)
        cond = 1;

    return cond;
}


/*
 *  performs integral division to avoid an exception on the host
 */
static sx_t dodivs(sx_t l, sx_t r, int mod)
{
    return (!mod)?
        ((xe(l, xmns) && xe(r, x_I))? xmns: xds(l, r)):
        ((xe(l, xmns) && xe(r, x_I))? xO: xrs(l, r));
}

#undef CHOOSEFPV


/*
 *  inspects if u is greater than 1 and is a power of 2
 */
static int ispow2(ux_t u)
{
    int n;

    if (xe(xba(u, xsu(u, xI)), xO))
        for (n = 0; xne(u, xO); u = xsrl(u, 1), n++)
            if (xt(xba(u, xI)))
                return n;

    return 0;
}


/*
 *  generates a symbol tree to replace symbol + integer
 */
static tree_t *addrtree(tree_t *e, sx_t n, ty_t *ty, tree_pos_t *tpos)
{
    sym_t *p, *q;

    assert(e);
    assert(ty);
    assert(tpos);
    assert(ir_cur);

    p = e->u.sym;
    assert(TY_ISPTR(ty) || TY_ISARRAY(ty));
    q = sym_new(SYM_KADDR, p, TY_UNQUAL(ty)->type);
    sym_ref(q, 1);
    if (p->scope == SYM_SGLOBAL || p->sclass == LEX_STATIC || p->sclass == LEX_EXTERN)
        ir_cur->symaddr(q, p, xns(n));
    else {
        stmt_t *cp;
        stmt_local(p);
        cp = stmt_new(STMT_ADDRESS);
        cp->u.addr.sym = q;
        cp->u.addr.base = p;
        cp->u.addr.offset = xns(n);
    }
    q->u.bt = e;
    e = tree_new(e->op, ty, NULL, NULL, tpos);
    e->u.sym = q;

    return e;
}


/*
 *  finds the base tree of an address symbol if possible
 */
tree_t *(simp_basetree)(const sym_t *p, tree_t *t)
{
    assert(p || t);

    if (!p)
        p = t->u.sym;
    if (p->f.computed)
        for (t = p->u.bt; t->u.sym->f.computed; t = t->u.sym->u.bt)
            continue;

    return t;
}


/*
 *  parses an integer constant expression;
 *  this includes parsing a constant expression:
 *      constant-exp:
 *          assign-exp
 *  instead of:
 *      constant-exp:
 *          cond-exp
 *  and any assignment is treated as a semantic error;
 *  ASSUMPTION: unsigned integers are compatible with signed ones on the host
 */
tree_t *(simp_intexpr)(int tok, sx_t *n, int ovf, ux_t m, const char *name, const lmap_t *posm)
{
    tree_t *p;

    assert(name);

    simp_needconst++;
    p = expr_asgn(tok, 0, 1, posm);
    simp_needconst--;

    if (!p)
        return NULL;

    if (op_generic(p->op) == OP_CNST && OP_ISINT(p->op)) {
        if (p->f.npce & (TREE_FCOMMA|TREE_FICE))
            err_dpos(TREE_TW(p), ERR_EXPR_NOINTCONSTW, name);
        if (ovf && n && ((TY_ISUNSIGN(p->type) && xgu(p->u.v.u, m)) ||
                         (!TY_ISUNSIGN(p->type) && xgs(p->u.v.s, xO) && xgu(p->u.v.u, m))))
            err_dpos(TREE_TW(p), ERR_EXPR_LARGEVAL, name, *n);
        else if (n)
            *n = p->u.v.s;
    } else {
        err_dpos(TREE_TW(p), ERR_EXPR_NOINTCONST, name);
        p = NULL;
    }

    return p;
}


/*
 *  generates a tree simplifying it;
 *  ASSUMPTION: pointers are uniform;
 *  ASSUMPTION: indexing is limited by ssz_t;
 *  ASSUMPTION: long double can represent all integers even if inexactly;
 *  ASSUMPTION: UB not triggered by an intermediate out-of-range pointer;
 *  ASSUMPTION: ~ of signed integer gives the same result on the host and target;
 *  ASSUMPTION: fp types of the host are same as those of the target;
 *  ASSUMPTION: -(-fp value) is equal to the original w/o other effects;
 *  ASSUMPTION: 2sC for signed integers assumed;
 *  ASSUMPTION: -(-min of signed integer) is equal to the original on the host and target;
 *  ASSUMPTION: negative value can be generated by left-shifting of ~0;
 *  ASSUMPTION: NPC of pointer type is represented as 0
 */
static tree_t *simplify(int op, ty_t *ty, tree_t *l, tree_t *r, tree_pos_t *tpos)
{
    int n;
    int sfx;
    ty_t *oty;
    tree_t *p;

    assert(ty);
    assert(l);
    assert(tpos);
    assert(ty_floattype);    /* ensures types initialized */
    assert(op_type(op) == 0);

    switch(op_scode(op)) {    /* TY_* */
        case 0:
            oty = TY_RMQENUM(ty);
            break;
        case TY_FLOAT:
            oty = ty_floattype;
            break;
        case TY_DOUBLE:
            oty = ty_doubletype;
            break;
        case TY_LDOUBLE:
            oty = ty_ldoubletype;
            break;
        case TY_INT:
            oty = ty_inttype;
            break;
        case TY_UNSIGNED:
            oty = ty_unsignedtype;
            break;
        case TY_LONG:
            oty = ty_longtype;
            break;
#ifdef SUPPORT_LL
        case TY_LLONG:
            oty = ty_longtype;
            break;
#endif    /* SUPPORT_LL */
        case TY_ULONG:
            oty = ty_ulongtype;
            break;
#ifdef SUPPORT_LL
        case TY_ULLONG:
            oty = ty_ullongtype;
            break;
#endif    /* SUPPORT_LL */
        case TY_POINTER:
            oty = ty_voidptype;
            break;
        default:
            assert(!"invalid type operator -- should never reach here");
            break;
    }

    sfx = op_sfx(oty);
    op = op_generic(op)+sfx;

    switch(op_optype(op)) {
        /* ADD */
        case OP_ADD+OP_F:
            switch(oty->op) {
                case TY_FLOAT:
                    foldbinov(f, F, xa, -TG_FLT_MAX, TG_FLT_MAX, 0);
                    break;
                case TY_DOUBLE:
                    foldbinov(d, D, xa, -TG_DBL_MAX, TG_DBL_MAX, 0);
                    break;
                case TY_LDOUBLE:
                    foldbinov(ld, X, xa, -TG_LDBL_MAX, TG_LDBL_MAX, 0);
                    break;
                default:
                    assert(!"invalid type operator -- should never reach here");
                    break;
            }
            commute(r, l);
            break;
        case OP_ADD+OP_I:
            switch(oty->op) {
                case TY_INT:
                    foldbinov(s, SI, xa, TG_INT_MIN, TG_INT_MAX, simp_needconst);
                    break;
                case TY_LONG:
                    foldbinov(s, SL, xa, TG_LONG_MIN, TG_LONG_MAX, simp_needconst);
                    break;
#ifdef SUPPORT_LL
                case TY_LLONG:
                    foldbinov(s, SLL, xa, TG_LLONG_MIN, TG_LLONG_MAX, simp_needconst);
                    break;
#endif    /* SUPPORT_LL */
                default:
                    assert(!"invalid type operator -- should never reach here");
                    break;
            }
            commute(r, l);
            identity(r, l, I, s, xO);
            break;
        case OP_ADD+OP_U:
            switch(oty->op) {
                case TY_UNSIGNED:
                    foldbinnv(xa, UI);
                    break;
                case TY_ULONG:
                    foldbinnv(xa, UL);
                    break;
#ifdef SUPPORT_LL
                case TY_ULLONG:
                    foldbinnv(xa, ULL);
                    break;
#endif    /* SUPPORT_LL */
                default:
                    assert(!"invalid type operator -- should never reach here");
                    break;
            }
            commute(r, l);
            identity(r, l, U, u, xO);
            break;
        case OP_ADD+OP_P:
            foldaddp(l, r, I, s);
            foldaddp(l, r, U, u);
            foldaddp(r, l, I, s);
            foldaddp(r, l, U, u);
            commute(r, l);
            identity(r, tree_retype(l, ty, NULL), I, s, xO);
            identity(r, tree_retype(l, ty, NULL), U, u, xO);
            if (OP_ISADDR(l->op) && op_generic(r->op) == OP_CNST) {
                assert(OP_ISINT(r->op));
                return addrtree(l, r->u.v.s, ty, tpos);
            }
            if (op_optype(l->op) == OP_ADD+OP_P && OP_ISADDR(l->kid[1]->op) &&
                op_generic(r->op) == OP_CNST) {
                assert(OP_ISINT(r->op));
                return simp_tree(OP_ADD + TY_POINTER, ty,
                                 l->kid[0], addrtree(l->kid[1], r->u.v.s, ty, tpos), tpos);
            }
            if (op_optype(l->op) == OP_SUB+OP_P && OP_ISADDR(l->kid[0]->op) &&
                op_generic(r->op) == OP_CNST) {
                assert(OP_ISINT(r->op));
                return simp_tree(OP_SUB + TY_POINTER, ty,
                                 addrtree(l->kid[0], r->u.v.s, ty, tpos), l->kid[1], tpos);
            }
            if (OP_ISADDR(r->op) && (op_generic(l->op) == OP_ADD || op_generic(l->op) == OP_SUB) &&
                op_generic(l->kid[1]->op) == OP_CNST) {
                assert(OP_ISINT(l->op) && OP_ISINT(l->kid[1]->op));
                return simp_tree(OP_ADD + TY_POINTER, ty,
                                 l->kid[0], simp_tree(op_generic(l->op) + TY_POINTER, ty,
                                                      r, l->kid[1], tpos), tpos);
            }
            if (OP_ISADDR(r->op) && op_generic(l->op) == OP_SUB &&
                op_generic(l->kid[0]->op) == OP_CNST) {
                assert(OP_ISINT(l->op) && OP_ISINT(l->kid[0]->op));
                return simp_tree(OP_SUB + TY_POINTER, ty,
                                 simp_tree(OP_ADD + TY_POINTER, ty,
                                           r, l->kid[0], tpos), l->kid[1], tpos);
            }
            if (op_optype(l->op) == OP_ADD+OP_P && op_generic(l->kid[1]->op) == OP_CNST &&
                op_generic(r->op) == OP_CNST) {
                return simp_tree(OP_ADD + TY_POINTER, ty, l->kid[0],
                           simp_tree(OP_ADD, l->kid[1]->type, l->kid[1],
                               (OP_ISINT(l->kid[1]->op))?
                                   enode_cast(r, l->kid[1]->type, 0, NULL): r, tpos), tpos);
            }
            /* (ptr - const) always turns into (ptr + const) */
            if (op_optype(l->op) == OP_SUB+OP_P && op_generic(l->kid[0]->op) == OP_CNST &&
                op_generic(r->op) == OP_CNST) {
                return simp_tree(OP_SUB + TY_POINTER, ty,
                                 simp_tree(OP_ADD, ty, r, l->kid[0], tpos), l->kid[1], tpos);
            }
            assert(l->op != OP_RIGHT || (!l->kid[0] && l->kid[1]) || l->kid[1]);
            if (l->op == OP_RIGHT && l->kid[1]) {
                return tree_right(l->kid[0],
                                  simp_tree(OP_ADD + TY_POINTER, ty, l->kid[1], r, tpos),
                                  ty, tpos);
            }
            break;
        /* SUB */
        case OP_SUB+OP_F:
            switch(oty->op) {
                case TY_FLOAT:
                    foldbinov(f, F, xs, -TG_FLT_MAX, TG_FLT_MAX, 0);
                    break;
                case TY_DOUBLE:
                    foldbinov(d, D, xs, -TG_DBL_MAX, TG_DBL_MAX, 0);
                    break;
                case TY_LDOUBLE:
                    foldbinov(ld, X, xs, -TG_LDBL_MAX, TG_LDBL_MAX, 0);
                    break;
                default:
                    assert(!"invalid type operator -- should never reach here");
                    break;
            }
            break;
        case OP_SUB+OP_I:
            switch(oty->op) {
                case TY_INT:
                    foldbinov(s, SI, xs, TG_INT_MIN, TG_INT_MAX, simp_needconst);
                    break;
                case TY_LONG:
                    foldbinov(s, SL, xs, TG_LONG_MIN, TG_LONG_MAX, simp_needconst);
                    break;
#ifdef SUPPORT_LL
                case TY_LLONG:
                    foldbinov(s, SLL, xs, TG_LLONG_MIN, TG_LLONG_MAX, simp_needconst);
                    break;
#endif    /* SUPPORT_LL */
                default:
                    assert(!"invalid type operator -- should never reach here");
                    break;
            }
            identity(r, l, I, s, xO);
            break;
        case OP_SUB+OP_U:
            switch(oty->op) {
                case TY_UNSIGNED:
                    foldbinnv(xs, UI);
                    break;
                case TY_ULONG:
                    foldbinnv(xs, UL);
                    break;
#ifdef SUPPORT_LL
                case TY_ULLONG:
                    foldbinnv(xs, ULL);
                    break;
#endif    /* SUPPORT_LL */
                default:
                    assert(!"invalid type operator -- should never reach here");
                    break;
            }
            identity(r, l, U, u, xO);
            break;
        case OP_SUB+OP_P:
            assert(OP_ISINT(r->op));
            /* adopted from ADD+P */
            if (OP_ISADDR(l->op) && (op_generic(r->op) == OP_ADD || op_generic(r->op) == OP_SUB) &&
                op_generic(r->kid[1]->op) == OP_CNST) {
                assert(OP_ISINT(r->kid[1]->op));
                return simp_tree(((op_generic(r->op) == OP_ADD)? OP_SUB: OP_ADD) + TY_POINTER, ty,
                                 simp_tree(OP_SUB + TY_POINTER, ty, l, r->kid[1], tpos),
                                 r->kid[0], tpos);
            }
            if (OP_ISADDR(l->op) && op_generic(r->op) == OP_SUB &&
                op_generic(r->kid[0]->op) == OP_CNST) {
                assert(OP_ISINT(r->kid[0]->op));
                return simp_tree(OP_ADD + TY_POINTER, ty,
                                 simp_tree(OP_SUB + TY_POINTER, ty, l, r->kid[0], tpos),
                                 r->kid[1], tpos);
            }
            /* until here */
            if (op_generic(r->op) == OP_CNST) {
                return simp_tree(OP_ADD + TY_POINTER, ty, l,
                                 tree_sconst((op_optype(r->op) == OP_CNST+OP_I)?
                                                 xn(r->u.v.s):
                                                 xn(xcts(r->u.v.u)), ty_ptrsinttype, tpos), tpos);
            }
            break;
        /* MUL */
        case OP_MUL+OP_F:
            switch(oty->op) {
                case TY_FLOAT:
                    foldbinov(f, F, xm, -TG_FLT_MAX, TG_FLT_MAX, 0);
                    break;
                case TY_DOUBLE:
                    foldbinov(d, D, xm, -TG_DBL_MAX, TG_DBL_MAX, 0);
                    break;
                case TY_LDOUBLE:
                    foldbinov(ld, X, xm, -TG_LDBL_MAX, TG_LDBL_MAX, 0);
                    break;
                default:
                    assert(!"invalid type operator -- should never reach here");
                    break;
            }
            commute(l, r);
            break;
        case OP_MUL+OP_I:
            switch(oty->op) {
                case TY_INT:
                    foldbinov(s, SI, xm, TG_INT_MIN, TG_INT_MAX, simp_needconst);
                    break;
                case TY_LONG:
                    foldbinov(s, SL, xm, TG_LONG_MIN, TG_LONG_MAX, simp_needconst);
                    break;
#ifdef SUPPORT_LL
                case TY_LLONG:
                    foldbinov(s, SLL, xm, TG_LLONG_MIN, TG_LLONG_MAX, simp_needconst);
                    break;
#endif    /* SUPPORT_LL */
                default:
                    assert(!"invalid type operator -- should never reach here");
                    break;
            }
            commute(l, r);
            identity(l, r, I, s, xI);
            noeffect(l, r, s, xO, tree_sconst(xO, ty, tpos), 0);
            distribute(ADD);
            distribute(SUB);
            stoshift(l, r, LSH);
            break;
        case OP_MUL+OP_U:
            switch(oty->op) {
                case TY_UNSIGNED:
                    foldbinnv(xm, UI);
                    break;
                case TY_ULONG:
                    foldbinnv(xm, UL);
                    break;
#ifdef SUPPORT_LL
                case TY_ULLONG:
                    foldbinnv(xm, ULL);
                    break;
#endif    /* SUPPORT_LL */
                default:
                    assert(!"invalid type operator -- should never reach here");
                    break;
            }
            commute(l, r);
            identity(l, r, U, u, xI);
            noeffect(l, r, u, xO, tree_uconst(xO, ty, tpos), 0);
            distribute(ADD);
            distribute(SUB);
            utoshift(l, r, LSH);
            break;
        /* DIV */
        case OP_DIV+OP_F:
            switch(oty->op) {
                case TY_FLOAT:
                    chkdivby0(f, 0);
                    foldbinov(f, F, xd, -TG_FLT_MAX, TG_FLT_MAX, 0);
                    break;
                case TY_DOUBLE:
                    chkdivby0(d, 0);
                    foldbinov(d, D, xd, -TG_DBL_MAX, TG_DBL_MAX, 0);
                    break;
                case TY_LDOUBLE:
                    chkdivby0(ld, 0);
                    foldbinov(ld, X, xd, -TG_LDBL_MAX, TG_LDBL_MAX, 0);
                    break;
                default:
                    assert(!"invalid type operator -- should never reach here");
                    break;
            }
            break;
        case OP_DIV+OP_I:
            chkdivby0(s, xO);
            switch(oty->op) {
                case TY_INT:
                    foldbinov(s, SI, xd, TG_INT_MIN, TG_INT_MAX, simp_needconst);
                    break;
                case TY_LONG:
                    foldbinov(s, SL, xd, TG_LONG_MIN, TG_LONG_MAX, simp_needconst);
                    break;
#ifdef SUPPORT_LL
                case TY_LLONG:
                    foldbinov(s, SLL, xd, TG_LLONG_MIN, TG_LLONG_MAX, simp_needconst);
                    break;
#endif    /* SUPPORT_LL */
                default:
                    assert(!"invalid type operator -- should never reach here");
                    break;
            }
            identity(r, l, I, s, xI);
            break;
        case OP_DIV+OP_U:
            chkdivby0(u, xO);
            switch(oty->op) {
                case TY_UNSIGNED:
                    foldbinnv(xd, UI);
                    break;
                case TY_ULONG:
                    foldbinnv(xd, UL);
                    break;
#ifdef SUPPORT_LL
                case TY_ULLONG:
                    foldbinnv(xd, ULL);
                    break;
#endif    /* SUPPORT_LL */
                default:
                    assert(!"invalid type operator -- should never reach here");
                    break;
            }
            identity(r, l, U, u, xI);
            utoshift(r, l, RSH);
            break;
        /* MOD */
        case OP_MOD+OP_I:
            chkdivby0(s, xO);
            switch(oty->op) {
                case TY_INT:
                    foldbinov(s, SI, xr, TG_INT_MIN, TG_INT_MAX, simp_needconst);
                    break;
                case TY_LONG:
                    foldbinov(s, SL, xr, TG_LONG_MIN, TG_LONG_MAX, simp_needconst);
                    break;
#ifdef SUPPORT_LL
                case TY_LLONG:
                    foldbinov(s, SLL, xr, TG_LLONG_MIN, TG_LLONG_MAX, simp_needconst);
                    break;
#endif    /* SUPPORT_LL */
                default:
                    assert(!"invalid type operator -- should never reach here");
                    break;
            }
            break;
        case OP_MOD+OP_U:
            chkdivby0(u, xO);
            switch(oty->op) {
                case TY_UNSIGNED:
                    foldbinnv(xr, UI);
                    break;
                case TY_ULONG:
                    foldbinnv(xr, UL);
                    break;
#ifdef SUPPORT_LL
                case TY_ULLONG:
                    foldbinnv(xr, ULL);
                    break;
#endif    /* SUPPORT_LL */
                default:
                    assert(!"invalid type operator -- should never reach here");
                    break;
            }
            noeffect(r, l, u, xI, tree_uconst(xO, ty, tpos), 0);
            if (r->op == OP_CNST+sfx && ispow2(r->u.v.u)) {
                return tree_bit(OP_BAND, l, tree_uconst(xsu(r->u.v.u, xI), oty, tpos), ty, tpos);
            }
            break;
        /* BAND */
        case OP_BAND+OP_U:
            switch(oty->op) {
                case TY_UNSIGNED:
                    foldbinnv(xba, UI);
                    break;
                case TY_ULONG:
                    foldbinnv(xba, UL);
                    break;
#ifdef SUPPORT_LL
                case TY_ULLONG:
                    foldbinnv(xba, ULL);
                    break;
#endif    /* SUPPORT_LL */
                default:
                    assert(!"invalid type operator -- should never reach here");
                    break;
            }
            commute(r, l);
            identity(r, l, U, u, oty->u.sym->u.lim.max.u);
            noeffect(r, l, u, xO, tree_uconst(xO, ty, tpos), 0);
            break;
        /* BOR */
        case OP_BOR+OP_U:
            switch(oty->op) {
                case TY_UNSIGNED:
                    foldbinnv(xbo, UI);
                    break;
                case TY_ULONG:
                    foldbinnv(xbo, UL);
                    break;
#ifdef SUPPORT_LL
                case TY_ULLONG:
                    foldbinnv(xbo, ULL);
                    break;
#endif    /* SUPPORT_LL */
                default:
                    assert(!"invalid type operator -- should never reach here");
                    break;
            }
            commute(r, l);
            identity(r, l, U, u, xO);
            break;
        /* BXOR */
        case OP_BXOR+OP_U:
            switch(oty->op) {
                case TY_UNSIGNED:
                    foldbinnv(xbx, UI);
                    break;
                case TY_ULONG:
                    foldbinnv(xbx, UL);
                    break;
#ifdef SUPPORT_LL
                case TY_ULLONG:
                    foldbinnv(xbx, ULL);
                    break;
#endif    /* SUPPORT_LL */
                default:
                    assert(!"invalid type operator -- should never reach here");
                    break;
            }
            commute(r, l);
            identity(r, l, U, u, xO);
            break;
        /* BCOM */
        case OP_BCOM+OP_I:    /* to be replaced by BCOM+U */
            switch(oty->op) {
                case TY_INT:
                    folduni(tree_sconst(SYM_CROPSI(xbc(l->u.v.s)), ty, tpos));
                    break;
                case TY_LONG:
                    folduni(tree_sconst(SYM_CROPSL(xbc(l->u.v.s)), ty, tpos));
                    break;
#ifdef SUPPORT_LL
                case TY_LLONG:
                    folduni(tree_sconst(SYM_CROPSLL(xbc(l->u.v.s)), ty, tpos));
                    break;
#endif    /* SUPPORT_LL */
                default:
                    assert(!"invalid type operator -- should never reach here");
                    break;
            }
            op = OP_BCOM+op_sfx(ty_ucounter(oty));
            idempotent(op);
            break;
        case OP_BCOM+OP_U:
            switch(oty->op) {
                case TY_UNSIGNED:
                    folduni(tree_uconst(SYM_CROPUI(xbc(l->u.v.u)), ty, tpos));
                    break;
                case TY_ULONG:
                    folduni(tree_uconst(SYM_CROPUL(xbc(l->u.v.u)), ty, tpos));
                    break;
#ifdef SUPPORT_LL
                case TY_ULLONG:
                    folduni(tree_uconst(SYM_CROPULL(xbc(l->u.v.u)), ty, tpos));
                    break;
#endif    /* SUPPORT_LL */
                default:
                    assert(!"invalid type operator -- should never reach here");
                    break;
            }
            idempotent(OP_BCOM+sfx);
            break;
        /* OP_NEG */
        case OP_NEG+OP_F:
            switch(oty->op) {
                case TY_FLOAT:
                    folduni(tree_fpconst(-l->u.v.f, ty, tpos));
                    break;
                case TY_DOUBLE:
                    folduni(tree_fpconst(-l->u.v.d, ty, tpos));
                    break;
                case TY_LDOUBLE:
                    folduni(tree_fpconst(-l->u.v.ld, ty, tpos));
                    break;
                default:
                    assert(!"invalid type operator -- should never reach here");
                    break;
            }
            idempotent(OP_NEG+sfx);
            break;
        case OP_NEG+OP_I:
            switch(oty->op) {
                case TY_INT:
                    folduni(tree_sconst(SYM_CROPSI(xn(l->u.v.s)), ty, tpos));
                    break;
                case TY_LONG:
                    folduni(tree_sconst(SYM_CROPSL(xn(l->u.v.s)), ty, tpos));
                    break;
#ifdef SUPPORT_LL
                case TY_LLONG:
                    folduni(tree_sconst(SYM_CROPSLL(xn(l->u.v.s)), ty, tpos));
                    break;
#endif    /* SUPPORT_LL */
                default:
                    assert(!"invalid type operator -- should never reach here");
                    break;
            }
            idempotent(OP_NEG+sfx);
            break;
        /* LSH */
        case OP_LSH+OP_I:
            warnovsh(1);
            warnovshcv();
            warnneglsh();
            foldlshov();
            identity(r, l, I, s, xO);
            break;
        case OP_LSH+OP_U:
            warnovsh(0);
            warnovshcv();
            switch(oty->op) {
                case TY_UNSIGNED:
                    foldshnv(xsl, UI);
                    break;
                case TY_ULONG:
                    foldshnv(xsl, UL);
                    break;
#ifdef SUPPORT_LL
                case TY_ULLONG:
                    foldshnv(xsl, ULL);
                    break;
#endif    /* SUPPORT_LL */
                default:
                    assert(!"invalid type operator -- should never reach here");
                    break;
            }
            identity(r, l, I, u, xO);
            break;
        /* RSH */
        case OP_RSH+OP_I:
            warnovsh(0);
            warnovshcv();
            warnnegrsh();
            foldrshov();
            identity(r, l, I, s, xO);
            break;
        case OP_RSH+OP_U:
            warnovsh(0);
            warnovshcv();
            switch(oty->op) {
                case TY_UNSIGNED:
                    foldshnv(xsrl, UI);
                    break;
                case TY_ULONG:
                    foldshnv(xsrl, UL);
                    break;
#ifdef SUPPORT_LL
                case TY_ULLONG:
                    foldshnv(xsrl, ULL);
                    break;
#endif    /* SUPPORT_LL */
                default:
                    assert(!"invalid type operator -- should never reach here");
                    break;
            }
            identity(r, l, I, u, xO);
            break;
        /* AND */
        case OP_AND+OP_I:    /* meaningless I */
            op = OP_AND;
            foldlog(xf(l->u.v.s));
            break;
        /* OR */
        case OP_OR+OP_I:    /* meaningless I */
            op = OP_OR;
            foldlog(xt(l->u.v.s));
            break;
        /* EQ */
        case OP_EQ+OP_F:
            switch(oty->op) {
                case TY_FLOAT:
                    foldcmp(f, xe);
                    break;
                case TY_DOUBLE:
                    foldcmp(d, xe);
                    break;
                case TY_LDOUBLE:
                    foldcmp(ld, xe);
                    break;
                default:
                    assert(!"invalid type operator -- should never reach here");
                    break;
            }
            commute(r, l);
            break;
        case OP_EQ+OP_I:
            foldcmp(s, xe);
            commute(r, l);
            zerofield(EQ, s);
            break;
        case OP_EQ+OP_U:    /* to be replaced by EQ+I */
            foldcmp(u, xe);
            commute(r, l);
            zerofield(EQ, u);
            symcmpz(0);
            op = OP_EQ+op_sfx(ty_scounter(oty));
            break;
        /* NE */
        case OP_NE+OP_F:
            switch(oty->op) {
                case TY_FLOAT:
                    foldcmp(f, xne);
                    break;
                case TY_DOUBLE:
                    foldcmp(d, xne);
                    break;
                case TY_LDOUBLE:
                    foldcmp(ld, xne);
                    break;
                default:
                    assert(!"invalid type operator -- should never reach here");
                    break;
            }
            break;
        case OP_NE+OP_I:
            foldcmp(s, xne);
            commute(r, l);
            zerofield(NE, s);
            break;
        case OP_NE+OP_U:    /* to be replaced by NE+I */
            foldcmp(u, xne);
            commute(r, l);
            zerofield(NE, u);
            symcmpz(1);
            op = OP_NE+op_sfx(ty_scounter(oty));
            break;
        /* GE */
        case OP_GE+OP_F:
            switch(oty->op) {
                case TY_FLOAT:
                    foldcmp(f, xge);
                    break;
                case TY_DOUBLE:
                    foldcmp(d, xge);
                    break;
                case TY_LDOUBLE:
                    foldcmp(ld, xge);
                    break;
                default:
                    assert(!"invalid type operator -- should never reach here");
                    break;
            }
            break;
        case OP_GE+OP_I:
            foldcmp(s, xge);
            break;
        case OP_GE+OP_U:
            foldcmp(u, xge);
            geu(l, r, 1);
            utoeq(l, EQ);
            break;
        /* GT */
        case OP_GT+OP_F:
            switch(oty->op) {
                case TY_FLOAT:
                    foldcmp(f, xg);
                    break;
                case TY_DOUBLE:
                    foldcmp(d, xg);
                    break;
                case TY_LDOUBLE:
                    foldcmp(ld, xg);
                    break;
                default:
                    assert(!"invalid type operator -- should never reach here");
                    break;
            }
            break;
        case OP_GT+OP_I:
            foldcmp(s, xg);
            break;
        case OP_GT+OP_U:
            foldcmp(u, xg);
            geu(r, l, 0);
            utoeq(r, NE);
            break;
        /* LE */
        case OP_LE+OP_F:
            switch(oty->op) {
                case TY_FLOAT:
                    foldcmp(f, xle);
                    break;
                case TY_DOUBLE:
                    foldcmp(d, xle);
                    break;
                case TY_LDOUBLE:
                    foldcmp(ld, xle);
                    break;
                default:
                    assert(!"invalid type operator -- should never reach here");
                    break;
            }
            break;
        case OP_LE+OP_I:
            foldcmp(s, xle);
            break;
        case OP_LE+OP_U:
            foldcmp(u, xle);
            geu(r, l, 1);
            utoeq(r, EQ);
            break;
        /* LT */
        case OP_LT+OP_F:
            switch(oty->op) {
                case TY_FLOAT:
                    foldcmp(f, xl);
                    break;
                case TY_DOUBLE:
                    foldcmp(d, xl);
                    break;
                case TY_LDOUBLE:
                    foldcmp(ld, xl);
                    break;
                default:
                    assert(!"invalid type operator -- should never reach here");
                    break;
            }
            break;
        case OP_LT+OP_I:
            foldcmp(s, xl);
            break;
        case OP_LT+OP_U:
            foldcmp(u, xl);
            geu(l, r, 0);
            utoeq(l, NE);
            break;
        /* NOT */
        case OP_NOT+OP_I:    /* meaningless I */
            op = OP_NOT;
            folduni(tree_sconst(xis(xf(l->u.v.s)), ty_inttype, tpos));
            break;
        /* OP_POS */
        case OP_POS+OP_F:
        case OP_POS+OP_I:
        case OP_POS+OP_U:
            return tree_retype(l, NULL, tpos);
        default:
            assert(!"invalid operation code -- should never reach here");
            break;
    }

    return tree_new(op, ty, l, r, tpos);
}


/*
 *  merges the constant expression info after simplifying a tree
 */
tree_t *(simp_tree)(int op, ty_t *ty, tree_t *l, tree_t *r, tree_pos_t *tpos)
{
    tree_t *p = simplify(op, ty, l, r, tpos);

    if (op_generic(p->op) == OP_CNST || op_generic(p->op) == OP_ADDRG) {
        if (l)
            p->f.npce |= l->f.npce;
        if (r)
            p->f.npce |= r->f.npce;
    }

    if (op_generic(p->op) != op_generic(op) || (l && l != l->orgn) || (r && r != r->orgn)) {
        if (l)
            l = l->orgn;
        if (r)
            r = r->orgn;
        p->orgn = tree_new(op, ty, l, r, tpos);
    }

    return p;
}


/*
 *  generates a conversion tree simplifying it;
 *  ASSUMPTION: pointers are uniform;
 *  ASSUMPTION: long double can represent all integers even if inexactly;
 *  ASSUMPTION: fp types of the host are same as those of the target;
 *  ASSUMPTION: 2sC for signed integers assumed;
 *  ASSUMPTION: NPC of pointer type is represented as 0
 */
static tree_t *cvsimplify(int op, ty_t *fty, ty_t *tty, tree_t *l)
{
    tree_t *p;
    int sfx, top;

    assert(op_type(op) == 0);
    assert(fty);
    assert(l);
    assert(ty_floattype);    /* ensures types initialized */

    sfx = op_sfx(tty);
    top = TY_RMQENUM(tty)->op;

    switch(op+op_type(sfx)) {
        case OP_CVF+OP_F:    /* from float/double/ldouble */
            samesize();
            if (tty->size > fty->size)
                switch(fty->op) {
                    case TY_FLOAT:    /* float to ldouble */
                        cvtnv(p->u.v.ld = l->u.v.f);
                        break;
                    case TY_DOUBLE:    /* double to ldouble */
                        cvtnv(p->u.v.ld = l->u.v.d);
                        break;
                    default:
                        assert(!"invalid type operator -- should never reach here");
                        break;
                }
            else
                switch(top) {
                    case TY_FLOAT:    /* ldouble to float */
                        cvtov(ld, -TG_FLT_MAX, TG_FLT_MAX, p->u.v.f = l->u.v.ld);
                        break;
                    case TY_DOUBLE:    /* ldouble to double */
                        cvtov(ld, -TG_DBL_MAX, TG_DBL_MAX, p->u.v.d = l->u.v.ld);
                        break;
                    default:
                        assert(!"invalid type operator -- should never reach here");
                        break;
                }
            break;
        case OP_CVI+OP_I:    /* from char/short/int/long/llong */
            samesize();
            if (tty->size > fty->size) {    /* widens */
                cvtnv(p->u.v.s = l->u.v.s);
            } else    /* narrows */
                switch(top) {
                    case TY_CHAR:
                        cvtov(s, TG_SCHAR_MIN, TG_SCHAR_MAX, p->u.v.s = SYM_CROPSC(l->u.v.s));
                        break;
                    case TY_SHORT:
                        cvtov(s, TG_SHRT_MIN, TG_SHRT_MAX, p->u.v.s = SYM_CROPSS(l->u.v.s));
                        break;
                    case TY_INT:
                        cvtov(s, TG_INT_MIN, TG_INT_MAX, p->u.v.s = SYM_CROPSI(l->u.v.s));
                        break;
#ifdef SUPPORT_LL
                    case TY_LONG:
                        cvtov(s, TG_LONG_MIN, TG_LONG_MAX, p->u.v.s = SYM_CROPSL(l->u.v.s));
                        break;
#endif    /* SUPPORT_LL */
                    default:
                        assert(!"invalid type operator -- should never reach here");
                        break;
                }
            break;
        case OP_CVU+OP_U:    /* from uint/ulong/ullong */
            samesize();
            switch(top) {
                case TY_UNSIGNED:    /* to uint */
                    cvtnv(p->u.v.u = SYM_CROPUI(l->u.v.u));
                    break;
                case TY_ULONG:       /* to ulong */
                    cvtnv(p->u.v.u = SYM_CROPUL(l->u.v.u));
                    break;
#ifdef SUPPORT_LL
                case TY_ULLONG:      /* to ullong */
                    cvtnv(p->u.v.u = l->u.v.u);
                    break;
#endif    /* SUPPORT_LL */
                default:
                    assert(!"invalid type operator -- should never reach here");
                    break;
            }
            break;
        case OP_CVF+OP_I:    /* from ldouble */
            switch(top) {
                case TY_INT:    /* to int */
                    cvtov(ld, xcsf(TG_INT_MIN)-1.0L, xcsf(TG_INT_MAX)+1.0L,
                          p->u.v.s = SYM_CROPSI(xcfs(l->u.v.ld)));
                    break;
                case TY_LONG:    /* to long */
                    cvtov(ld, xcsf(TG_LONG_MIN)-1.0L, xcsf(TG_LONG_MAX)+1.0L,
                          p->u.v.s = SYM_CROPSL(xcfs(l->u.v.ld)));
                    break;
#ifdef SUPPORT_LL
                case TY_LLONG:    /* to llong */
                    cvtov(ld, xcsf(TG_LLONG_MIN)-1.0L, xcsf(TG_LLONG_MAX)+1.0L,
                          p->u.v.s = SYM_CROPSLL(xcfs(l->u.v.ld)));
                    break;
#endif    /* SUPPORT_LL */
                default:
                    assert(!"invalid type operator -- should never reach here");
                    break;
            }
            break;
        case OP_CVI+OP_F:    /* from int/long/llong */
            /* to ldouble */
            cvtnv(p->u.v.ld = xcsf(l->u.v.s));
            break;
        case OP_CVI+OP_U:    /* from uchar/ushort/int/long/llong */
            switch(top) {
                case TY_UNSIGNED:    /* to uint */
                    cvtnv(p->u.v.u = SYM_CROPUI(l->u.v.u));
                    break;
                case TY_ULONG:       /* to ulong */
                    cvtnv(p->u.v.u = SYM_CROPUL(l->u.v.u));
                    break;
#ifdef SUPPORT_LL
                case TY_ULLONG:      /* to ullong */
                    cvtnv(p->u.v.u = SYM_CROPULL(l->u.v.u));
                    break;
#endif    /* SUPPORT_LL */
            }
            break;
        case OP_CVU+OP_I:    /* from uint/ulong/ullong */
            switch(top) {
                case TY_CHAR:    /* to uchar */
                    cvtnv(p->u.v.u = SYM_CROPUC(l->u.v.u));
                    break;
                case TY_SHORT:    /* to short */
                    cvtnv(p->u.v.u = SYM_CROPUS(l->u.v.u));
                    break;
                case TY_INT:      /* to int */
                    cvtus(TG_INT_MAX, p->u.v.s = SYM_CROPSI(l->u.v.u));
                    break;
                case TY_LONG:    /* to long */
                    cvtus(TG_LONG_MAX, p->u.v.s = SYM_CROPSL(l->u.v.u));
                    break;
#ifdef SUPPORT_LL
                case TY_LLONG:    /* to llong */
                    cvtus(TG_LLONG_MAX, p->u.v.s = SYM_CROPSLL(l->u.v.u));
                    break;
#endif    /* SUPPORT_LL */
                default:
                    assert(!"invalid type operator -- should never reach here");
                    break;
            }
            break;
        case OP_CVU+OP_P:    /* from uint/ulong */
            cvtnv(p->u.v.p = l->u.v.u);
            break;
        case OP_CVP+OP_U:    /* from pointer */
            switch(top) {
                case TY_UNSIGNED:    /* to uint */
                    cvtnv(p->u.v.u = SYM_CROPUI(l->u.v.p));
                    break;
                case TY_ULONG:       /* to ulong */
                    cvtnv(p->u.v.u = SYM_CROPUL(l->u.v.p));
                    break;
                default:
                    assert(!"invalid type operator -- should never reach here");
                    break;
            }
            break;
        default:
            assert(!"invalid operation code -- should never reach here");
            break;
    }

    return tree_new(op+OP_CVSFX(fty, tty), tty, l, NULL, l->pos);
}


/*
 *  merges the constant expression info after simplifying a tree
 */
tree_t *(simp_cvtree)(int op, ty_t *fty, ty_t *tty, tree_t *l)
{
    tree_t *p = cvsimplify(op, fty, tty, l);

    if (op_generic(p->op) == OP_CNST || op_generic(p->op) == OP_ADDRG)
        p->f.npce |= l->f.npce;

    if (op_generic(p->op) != op_generic(op) || (l && l != l->orgn)) {
        if (l)
            l = l->orgn;
        p->orgn = tree_new(op, tty, l, NULL, l->pos);
    }

    return p;
}

/* end of simp.c */
