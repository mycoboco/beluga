/*
 *  constant folding and optimization
 */

#include <limits.h>        /* LONG_MIN */
#include <stddef.h>        /* NULL */
#include <stdlib.h>        /* ldiv, ldiv_t */
#include <cbl/assert.h>    /* assert */

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

#define addld_s addfp_s
#define addd_s  addfp_s
#define addf_s  addfp_s
#define subld_s subfp_s
#define subd_s  subfp_s
#define subf_s  subfp_s
#define mulld_s mulfp_s
#define muld_s  mulfp_s
#define mulf_s  mulfp_s
#define divld_s divfp_s
#define divd_s  divfp_s
#define divf_s  divfp_s

#define doaddld(l, op, r) ((l) op (r))
#define doaddd(l, op, r)  ((l) op (r))
#define doaddf(l, op, r)  ((l) op (r))
#define dosubld(l, op, r) ((l) op (r))
#define dosubd(l, op, r)  ((l) op (r))
#define dosubf(l, op, r)  ((l) op (r))
#define domulld(l, op, r) ((l) op (r))
#define domuld(l, op, r)  ((l) op (r))
#define domulf(l, op, r)  ((l) op (r))
#define dodivld(l, op, r) ((l) op (r))
#define dodivd(l, op, r)  ((l) op (r))
#define dodivf(l, op, r)  ((l) op (r))
#define doaddli(l, op, r) ((l) op (r))
#define dosubli(l, op, r) ((l) op (r))
#define domulli(l, op, r) (domulli((l), #op, (r)))
#define dodivli(l, op, r) (dodivli((l), #op, (r)))

#define SYM_CROPF
#define SYM_CROPD
#define SYM_CROPX

/* folds constants for binary operations with no overflow */
#define foldbinnv(OP, CROP)                                                  \
    if (l->op == OP_CNST+sfx && r->op == OP_CNST+sfx) {                      \
        return tree_uconst_s(SYM_CROP##CROP(l->u.v.ul OP r->u.v.ul), ty);    \
    }

/* folds constants for binary operations with overflow */
#define foldbinov(VAR, OP, CROP, FUNC, MIN, MAX, needconst)                        \
    if (l->op == OP_CNST+sfx && r->op == OP_CNST+sfx &&                            \
        FUNC##VAR##_s(l->u.v.VAR, r->u.v.VAR, MIN, MAX, needconst)) {              \
        p = tree_new_s(OP_CNST+sfx, ty, NULL, NULL);                               \
        p->u.v.VAR = SYM_CROP##CROP(do##FUNC##VAR(l->u.v.VAR, OP, r->u.v.VAR));    \
        return p;                                                                  \
    }

/* checks if dividend is 0 */
#define chkdivby0(VAR)                                \
    if (r->op == OP_CNST+sfx && r->u.v.VAR == 0) {    \
        err_issue_s(ERR_EXPR_DIVBYZERO);              \
        break;                                        \
    }

/* removes identities */
#define identity(X, Y, TYPE, VAR, VAL)                                   \
    if (op_optype(X->op) == OP_CNST+OP_##TYPE && X->u.v.VAR == VAL) {    \
        p = Y;                                                           \
        return p;                                                        \
    }

/* removes operations with no effect */
#define noeffect(X, Y, VAR, VAL, EXPR, RVAL)                   \
    if (X->op == OP_CNST+sfx && X->u.v.VAR == VAL) {           \
        err_issue_s(ERR_EXPR_NOEFFECT, (long)RVAL);            \
        return tree_right_s(tree_root_s(Y), EXPR, X->type);    \
    }

/* makes right child of tree have constant if any */
#define commute(L, R)                                                      \
    if (op_generic(R->op) == OP_CNST && op_generic(L->op) != OP_CNST) {    \
        tree_t *t = L; L = R; R = t;                                       \
    }

/* folds constants for comparison operations */
#define foldcmp(VAR, OP)                                               \
    if (l->op == OP_CNST+sfx && r->op == OP_CNST+sfx) {                \
        return tree_sconst_s(l->u.v.VAR OP r->u.v.VAR, ty_inttype);    \
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
        return p;                                                                  \
    }

/* folds constants for shift operations with no overflow */
#define foldshnv(OP, CROP)                                                               \
    if (l->op == OP_CNST+sfx && op_optype(r->op) == OP_CNST+OP_I && r->u.v.li >= 0 &&    \
        r->u.v.li < TG_CHAR_BIT*l->type->size) {                                         \
        return tree_uconst_s(SYM_CROP##CROP(l->u.v.ul OP r->u.v.li), ty);                \
    }

/* folds constants for left-shift operation with overflow */
#define foldlshov()                                                                     \
    if (l->op == OP_CNST+sfx && op_optype(r->op) == OP_CNST+OP_I &&                     \
        l->u.v.li >= 0 && r->u.v.li >= 0 && r->u.v.li < TG_CHAR_BIT*l->type->size &&    \
        mulli_s(l->u.v.li, 1L << r->u.v.li, oty->u.sym->u.lim.min.li,                   \
                oty->u.sym->u.lim.max.li, simp_needconst)) {                            \
        return tree_sconst_s(l->u.v.li << r->u.v.li, ty);                               \
    }

/* folds constans for right-shift operation with overflow;
   ASSUMPTION: logical right shift handled elsewhere */
#define foldrshov()                                                                      \
    if (l->op == OP_CNST+sfx && op_optype(r->op) == OP_CNST+OP_I && r->u.v.li >= 0 &&    \
        r->u.v.li < TG_CHAR_BIT*l->type->size) {                                         \
        long int n = l->u.v.li >> r->u.v.li;                                             \
        if (l->u.v.li < 0 && n > 0) {                                                    \
            n |= ~(~0UL >> r->u.v.li);                                                   \
            assert(n < 0);                                                               \
        }                                                                                \
        return tree_sconst_s(n, ty);                                                     \
    }

/* warns left shift of negative values */
#define warnneglsh()                                      \
    if (op_generic(l->op) == OP_CNST && l->u.v.li < 0)    \
        err_issue_s(ERR_EXPR_LSHIFTNEG);

/* warns right shift of negative values */
#define warnnegrsh()                                      \
    if (op_generic(l->op) == OP_CNST && l->u.v.li < 0)    \
        err_issue_s(ERR_EXPR_RSHIFTNEG);

/* warns overshifting */
#define warnovsh(sign)                                                 \
    if (op_optype(r->op) == OP_CNST+OP_I &&                            \
        (r->u.v.li < 0 || r->u.v.li >= TG_CHAR_BIT*l->type->size ||    \
         ((sign) && op_generic(l->op) == OP_CNST && l->u.v.li &&       \
          r->u.v.li >= TG_CHAR_BIT*l->type->size-1))) {                \
        err_issue_s(ERR_EXPR_OVERSHIFTS, r->u.v.li);                   \
        break;                                                         \
    }

/* warns overshifting for converted but unfolded operands */
#define warnovshcv()                                                 \
    if (OP_ISCV(r->op) && op_generic(r->kid[0]->op) == OP_CNST) {    \
        if (OP_ISSINT(r->kid[0]->op))                                \
            err_issue_s(ERR_EXPR_OVERSHIFTS, r->kid[0]->u.v.li);     \
        else if (OP_ISUINT(r->kid[0]->op))                           \
            err_issue_s(ERR_EXPR_OVERSHIFTU, r->kid[0]->u.v.ul);     \
        else                                                         \
            err_issue_s(ERR_EXPR_OVERSHIFT);                         \
        break;                                                       \
    }

/* folds constants for ADD+P operation;
   ASSUMPTION: pointer addition on the target can be emulated by integral addition */
#define foldaddp(L, R, RTYPE, VAR)                                           \
    if (L->op == OP_CNST+sfx && op_optype(R->op) == OP_CNST+OP_##RTYPE) {    \
        p = tree_new_s(OP_CNST+sfx, ty, NULL, NULL);                         \
        p->u.v.tp = L->u.v.tp + R->u.v.VAR;                                  \
        return p;                                                            \
    }

/* distributes constants to fold */
#define distribute(OP)                                                              \
    if (l->op == OP_CNST+sfx && r->op == OP_##OP+sfx &&                             \
        (r->kid[0]->op == OP_CNST+sfx || r->kid[1]->op == OP_CNST+sfx)) {           \
        return simp_tree_s(OP_##OP, ty, simp_tree_s(OP_MUL, oty, l, r->kid[0]),     \
                                        simp_tree_s(OP_MUL, oty, l, r->kid[1]));    \
    }

/* converts signed operation to shift;
   ASSUMPTION: 2sC for signed integers assumed */
#define stoshift(X, Y, OP)                                                          \
    if (X->op == OP_CNST+sfx && X->u.v.li > 0 && (n = ispow2(X->u.v.li)) != 0) {    \
        return simp_tree_s(OP_##OP, ty, Y, tree_sconst_s(n, ty_inttype));           \
    }

/* converts unsigned operation to shift */
#define utoshift(X, Y, OP)                                                   \
    if (X->op == OP_CNST+sfx && (n = ispow2(X->u.v.ul)) != 0) {              \
        return simp_tree_s(OP_##OP, ty, Y, tree_sconst_s(n, ty_inttype));    \
    }

/* gets rid of unnecessary bit extraction (w/o conversion from FIELD) */
#define zerofield(OP, VAR)                                                                   \
    if (l->op == OP_FIELD && r->op == OP_CNST+sfx && r->u.v.VAR == 0) {                      \
        assert(TY_ISINT(l->type) || TY_ISUNSIGNED(l->type));                                 \
        return tree_cmp_s(OP_##OP,                                                           \
                   tree_bit_s(OP_BAND, l->kid[0],                                            \
                       tree_uconst_s(SYM_FLDMASK(l->u.field) << SYM_FLDRIGHT(l->u.field),    \
                                     ty_unsignedtype), NULL),                                \
                   r, NULL);                                                                 \
    }

/* gets rid of unnecessary bit extraction (w/ conversion from FIELD) */
#define zerofieldc(OP, TYPE, VAR)                                               \
    if (OP_ISCV(l->op) && l->kid[0]->op == OP_FIELD &&                          \
        r->op == OP_CNST+sfx && r->u.v.VAR == 0) {                              \
        assert(TY_ISINT(l->kid[0]->type) || TY_ISUNSIGNED(l->kid[0]->type));    \
        return tree_cmp_s(OP_##OP,                                              \
                   tree_bit_s(OP_BAND, l->kid[0]->kid[0],                       \
                       tree_uconst_s(SYM_FLDMASK(l->kid[0]->u.field) <<         \
                                         SYM_FLDRIGHT(l->kid[0]->u.field),      \
                                     ty_unsignedtype), NULL),                   \
                   tree_uconst_s(0, ty_unsignedtype), NULL);                    \
    }

/* folds (in)equality comparison of symbol to zero */
#define symcmpz(V)                                                              \
    if (op_optype(l->op) == OP_CVP+OP_U && OP_ISADDR(l->kid[0]->op) &&          \
        op_generic(r->op) == OP_CNST && r->u.v.ul == 0) {                       \
        if (!l->kid[0]->u.sym->f.outofline) {                                   \
            p = tree_untype(simp_basetree(NULL, l->kid[0]));                    \
            err_issuep(&p->pos, ERR_EXPR_SYMBOLTRUE, p->u.sym, " a symbol");    \
        }                                                                       \
        p = tree_sconst_s(V, ty_inttype);                                       \
        p->f.npce |= (TREE_FADDR|TREE_FACE|TREE_FICE);                          \
        return p;                                                               \
    }

/* removes unsigned comparisons that always result in constants */
#define geu(L, R, V)                                                                      \
    if (R->op == OP_CNST+sfx && R->u.v.ul == 0) {                                         \
        err_issue_s(ERR_EXPR_UNSIGNEDCMP, (V)? "true": "false");                          \
        return tree_right_s(tree_root_s(L), tree_sconst_s(V, ty_inttype), ty_inttype);    \
    }

/* removes operations that are meaningless when applied twice */
#define idempotent(OP) if (l->op == OP) { return l->kid[0]; }

/* changes unsigned relational comparisons to equality comparisons */
#define utoeq(C, OP)                                 \
    if (C->op == OP_CNST+sfx && C->u.v.ul == 0) {    \
        return tree_cmp_s(OP_##OP, l, r, ty);        \
    }


/* removes conversion between similar types of same size */
#define samesize()                    \
    if (fty->size == tty->size) {     \
        p = tree_retype_s(l, tty);    \
        return p;                     \
    }

/* converts constant whose result type has no overflow */
#define cvtnv(EXPR)                                      \
    if (op_generic(l->op) == OP_CNST) {                  \
        p = tree_new_s(OP_CNST+sfx, tty, NULL, NULL);    \
        EXPR;                                            \
        return p;                                        \
    }

/* converts constant whose result type has overflow */
#define cvtov(VAR, MIN, MAX, EXPR)                                           \
    if (op_generic(l->op) == OP_CNST) {                                      \
        /* diagnostics issued in enode_cast_s() if necessary */              \
        if (simp_needconst || (l->u.v.VAR >= MIN && l->u.v.VAR <= MAX)) {    \
            p = tree_new_s(OP_CNST+sfx, tty, NULL, NULL);                    \
            EXPR;                                                            \
            return p;                                                        \
        }                                                                    \
    }

/* converts constant of unsigned type to signed */
#define cvtus(MAX, EXPR)                                           \
    if (op_generic(l->op) == OP_CNST) {                            \
        /* diagnostics issued in enode_cast_s() if necessary */    \
        if (simp_needconst || l->u.v.ul <= MAX) {                  \
            p = tree_new_s(OP_CNST+sfx, tty, NULL, NULL);          \
            EXPR;                                                  \
            return p;                                              \
        }                                                          \
    }


int simp_needconst;    /* > 0 while constant is needed */


/*
 *  checks if a signed integer expression overflows for addition
 */
static int addli_s(long l, long r, long min, long max, int needconst)
{
    int cond = (l == 0 || r == 0 ||
                (l < 0 && r > 0) ||
                (l > 0 && r < 0) ||
                (l < 0 && r < 0 && l >= min-r) ||
                (l > 0 && r > 0 && l <= max-r));
    if (!cond && needconst) {
        err_issue_s(ERR_EXPR_OVFCONST);
        cond = 1;
    }

    return cond;
}


/*
 *  checks if a floating-point expression overflows for addition;
 *  ASSUMPTION: fp addition on the host works in reasonable way;
 *  ASSUMPTION: rounding direction on the host is same as on the target
 */
static int addfp_s(long double l, long double r, long double min, long double max, int needconst)
{
    int cond = (l == 0 || r == 0 ||
                (l < 0 && r > 0) ||
                (l > 0 && r < 0) ||
                (l < 0 && r < 0 && l >= min-r) ||
                (l > 0 && r > 0 && l <= max-r));
    if (!cond)
        err_issue_s(ERR_EXPR_OVFCONSTFP);
    if (needconst)
        cond = 1;

    return cond;
}


/*
 *  checks if a signed integer expression overflows for subtraction
 */
static int subli_s(long l, long r, long min, long max, int needconst)
{
    int cond = (l == 0 || r == 0 ||
                (l < 0 && r < 0) ||
                (l > 0 && r > 0) ||
                (l < 0 && r > 0 && l >= min+r) ||
                (l > 0 && r < 0 && l <= max+r));
    if (!cond && needconst) {
        err_issue_s(ERR_EXPR_OVFCONST);
        cond = 1;
    }

    return cond;
}


/*
 *  checks if a floating-point expression overflows for subtraction;
 *  ASSUMPTION: fp subtraction on the host works in reasonable way;
 *  ASSUMPTION: rounding direction on the host is same as on the target
 */
static int subfp_s(long double l, long double r, long double min, long double max, int needconst)
{
    return addfp_s(l, -r, min, max, needconst);
}


/*
 *  checks if a signed integer expression overflows for multipication;
 *  ASSUMPTION: 2sC for signed integers assumed
 */
static int mulli_s(long l, long r, long min, long max, int needconst)
{
    int cond = (!(l == -1 && r == min) &&
                !(l == min && r == -1) &&
                ((l == 0 || r == 0) ||
                 (l < 0 && r < 0 && l >= ldiv(max, r).quot) ||
                 (l < 0 && r > 0 && l >= ldiv(min, r).quot) ||
                 (l > 0 && r < 0 && (r == -1 || l <= ldiv(min, r).quot)) ||
                 (l > 0 && r > 0 && l <= max/r)));
    if (!cond && needconst) {
        err_issue_s(ERR_EXPR_OVFCONST);
        cond = 1;
    }

    return cond;
}


/*
 *  checks if a floating-point expression overflows for multipication;
 *  ASSUMPTION: fp multiplication works in reasonable way;
 *  ASSUMPTION: rounding direction on the host is same as on the target
 */
static int mulfp_s(long double l, long double r, long double min, long double max, int needconst)
{
    int cond = ((l >= -1 && l <= 1) ||
                (r >= -1 && r <= 1) ||
                (l == 0 || r == 0) ||
                (l < 0 && r < 0 && l >= max/r) ||
                (l < 0 && r > 0 && l >= min/r) ||
                (l > 0 && r < 0 && l <= min/r) ||
                (l > 0 && r > 0 && l <= max/r));
    if (!cond)
        err_issue_s(ERR_EXPR_OVFCONSTFP);
    if (needconst)
        cond = 1;

    return cond;
}


/*
 *  performs integral multiplication to avoid an exception on the host
 */
static long (domulli)(long l, const char *op, long r)
{
    assert(op);
    assert(*op == '*');
#ifdef NDEBUG
    UNUSED(op);
#endif    /* NDEBUG */

    return ((l == -1 && r == LONG_MIN) || (l == LONG_MIN && r == -1))? LONG_MIN: l * r;
}


/*
 *  checks if a signed integer expression overflows for division;
 *  ASSUMPTION: 2sC for signed integers assumed
 */
static int divli_s(long l, long r, long min, long max, int needconst)
{
    int cond = !(l == min && r == -1);

    UNUSED(max);

    if (!cond && needconst) {
        err_issue_s(ERR_EXPR_OVFCONST);
        cond = 1;
    }

    return cond;
}


/*
 *  checks if a floating-point expression overflows for division;
 *  ASSUMPTION: fp division works in reasonable way;
 *  ASSUMPTION: rounding direction on the host is same as on the target
 */
static int divfp_s(long double l, long double r, long double min, long double max, int needconst)
{
    int cond;

    UNUSED(min);

    if (l < 0)
        l = -l;
    if (r < 0)
        r = -r;
    cond = !(r < 1 && l > max*r);
    if (!cond)
        err_issue_s(ERR_EXPR_OVFCONSTFP);
    if (needconst)
        cond = 1;

    return cond;
}


/*
 *  performs integral division to avoid an exception on the host
 */
static long (dodivli)(long l, const char *op, long r)
{
    assert(op);
    assert(*op == '/' || *op == '%');

    if (*op == '/') {
        return (l == LONG_MIN && r == -1)? LONG_MIN: l / r;
    } else {
        assert(*op == '%');
        return (l == LONG_MIN && r == -1)? 0: l / r;
    }
}


/*
 *  inspects if u is greater than 1 and is a power of 2
 */
static int ispow2(unsigned long u)
{
    int n;

    if ((u & (u-1)) == 0)
        for (n = 0; u != 0; u >>= 1, n++)
            if (u & 1)
                return n;

    return 0;
}


/*
 *  generates a symbol tree to replace symbol + integer
 */
static tree_t *addrtree_s(tree_t *e, long n, ty_t *ty)
{
    sym_t *p, *q;

    assert(e);
    assert(ty);
    assert(ir_cur);
    UNUSED(n);

    p = e->u.sym;
    assert(TY_ISPTR(ty) || TY_ISARRAY(ty));
    q = sym_new(SYM_KADDR, p, TY_UNQUAL(ty)->type);
    sym_ref(q, 1);
    if (p->scope == SYM_SGLOBAL || p->sclass == LEX_STATIC || p->sclass == LEX_EXTERN)
        ir_cur->symaddr(q, p, n);
    else {
        stmt_t *cp;
        stmt_local(p);
        cp = stmt_new(STMT_ADDRESS);
        cp->u.addr.sym = q;
        cp->u.addr.base = p;
        cp->u.addr.offset = n;
    }
    q->u.bt = e;
    e = tree_new_s(e->op, ty, NULL, NULL);
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
 *  ASSUMPTION: unsigned long is compatible with signed one on the host
 */
tree_t *(simp_intexpr)(int tok, long *n, int ovf, const char *name)
{
    tree_t *p;

    assert(name);
    assert(ty_longtype);

    simp_needconst++;
    p = expr_asgn(tok, 0, 1);
    simp_needconst--;

    if (!p)
        return NULL;

    if (op_generic(p->op) == OP_CNST && OP_ISINT(p->op)) {
        if (p->f.npce & (TREE_FCOMMA|TREE_FICE))
            err_issuep(&p->pos, ERR_EXPR_NOINTCONSTW, name);
        if (ovf && TY_ISUNSIGN(p->type) && p->u.v.ul > TG_LONG_MAX)
            err_issuep(&p->pos, ERR_EXPR_LARGEVAL, name, *n);
        else if (n)
            *n = p->u.v.li;
    } else {
        err_issuep(&p->pos, ERR_EXPR_NOINTCONST, name);
        p = NULL;
    }

    return p;
}


/*
 *  generates a tree simplifying it;
 *  ASSUMPTION: pointers are uniform;
 *  ASSUMPTION: indexing is limited by long;
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
static tree_t *simplify_s(int op, ty_t *ty, tree_t *l, tree_t *r)
{
    int n;
    int sfx;
    ty_t *oty;
    tree_t *p;

    assert(ty);
    assert(l);
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
        case TY_ULONG:
            oty = ty_ulongtype;
            break;
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
                    foldbinov(f, +, F, add, -TG_FLT_MAX, TG_FLT_MAX, 0);
                    break;
                case TY_DOUBLE:
                    foldbinov(d, +, D, add, -TG_DBL_MAX, TG_DBL_MAX, 0);
                    break;
                case TY_LDOUBLE:
                    foldbinov(ld, +, X, add, -TG_LDBL_MAX, TG_LDBL_MAX, 0);
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
                    foldbinov(li, +, SI, add, TG_INT_MIN, TG_INT_MAX, simp_needconst);
                    break;
                case TY_LONG:
                    foldbinov(li, +, SL, add, TG_LONG_MIN, TG_LONG_MAX, simp_needconst);
                    break;
                default:
                    assert(!"invalid type operator -- should never reach here");
                    break;
            }
            commute(r, l);
            identity(r, l, I, li, 0);
            break;
        case OP_ADD+OP_U:
            switch(oty->op) {
                case TY_UNSIGNED:
                    foldbinnv(+, UI);
                    break;
                case TY_ULONG:
                    foldbinnv(+, UL);
                    break;
                default:
                    assert(!"invalid type operator -- should never reach here");
                    break;
            }
            commute(r, l);
            identity(r, l, U, ul, 0);
            break;
        case OP_ADD+OP_P:
            foldaddp(l, r, I, li);
            foldaddp(l, r, U, ul);
            foldaddp(r, l, I, li);
            foldaddp(r, l, U, ul);
            commute(r, l);
            identity(r, tree_retype_s(l, ty), I, li, 0);
            identity(r, tree_retype_s(l, ty), U, ul, 0);
            if (OP_ISADDR(l->op) && op_generic(r->op) == OP_CNST) {
                assert(OP_ISINT(r->op));
                return addrtree_s(l, r->u.v.li, ty);
            }
            if (op_optype(l->op) == OP_ADD+OP_P && OP_ISADDR(l->kid[1]->op) &&
                op_generic(r->op) == OP_CNST) {
                assert(OP_ISINT(r->op));
                return simp_tree_s(OP_ADD + TY_POINTER, ty, l->kid[0],
                                                            addrtree_s(l->kid[1], r->u.v.li, ty));
            }
            if (op_optype(l->op) == OP_SUB+OP_P && OP_ISADDR(l->kid[0]->op) &&
                op_generic(r->op) == OP_CNST) {
                assert(OP_ISINT(r->op));
                return simp_tree_s(OP_SUB + TY_POINTER, ty, addrtree_s(l->kid[0], r->u.v.li, ty),
                                                            l->kid[1]);
            }
            if (OP_ISADDR(r->op) && (op_generic(l->op) == OP_ADD || op_generic(l->op) == OP_SUB) &&
                op_generic(l->kid[1]->op) == OP_CNST) {
                assert(OP_ISINT(l->op) && OP_ISINT(l->kid[1]->op));
                return simp_tree_s(OP_ADD + TY_POINTER, ty, l->kid[0],
                                   simp_tree_s(op_generic(l->op) + TY_POINTER, ty, r, l->kid[1]));
            }
            if (OP_ISADDR(r->op) && op_generic(l->op) == OP_SUB &&
                op_generic(l->kid[0]->op) == OP_CNST) {
                assert(OP_ISINT(l->op) && OP_ISINT(l->kid[0]->op));
                return simp_tree_s(OP_SUB + TY_POINTER, ty,
                                   simp_tree_s(OP_ADD + TY_POINTER, ty, r, l->kid[0]), l->kid[1]);
            }
            if (op_optype(l->op) == OP_ADD+OP_P && op_generic(l->kid[1]->op) == OP_CNST &&
                op_generic(r->op) == OP_CNST) {
                return simp_tree_s(OP_ADD + TY_POINTER, ty, l->kid[0],
                           simp_tree_s(OP_ADD, l->kid[1]->type, l->kid[1],
                               (OP_ISINT(l->kid[1]->op))?
                                   enode_cast_s(r, l->kid[1]->type, 0): r));
            }
            /* (ptr - const) always turns into (ptr + const) */
            if (op_optype(l->op) == OP_SUB+OP_P && op_generic(l->kid[0]->op) == OP_CNST &&
                op_generic(r->op) == OP_CNST) {
                return simp_tree_s(OP_SUB + TY_POINTER, ty, simp_tree_s(OP_ADD, ty, r, l->kid[0]),
                                                            l->kid[1]);
            }
            assert(l->op != OP_RIGHT || (!l->kid[0] && l->kid[1]) || l->kid[1]);
            if (l->op == OP_RIGHT && l->kid[1]) {
                return tree_right_s(l->kid[0], simp_tree_s(OP_ADD + TY_POINTER, ty, l->kid[1], r),
                                    ty);
            }
            break;
        /* SUB */
        case OP_SUB+OP_F:
            switch(oty->op) {
                case TY_FLOAT:
                    foldbinov(f, -, F, sub, -TG_FLT_MAX, TG_FLT_MAX, 0);
                    break;
                case TY_DOUBLE:
                    foldbinov(d, -, D, sub, -TG_DBL_MAX, TG_DBL_MAX, 0);
                    break;
                case TY_LDOUBLE:
                    foldbinov(ld, -, X, sub, -TG_LDBL_MAX, TG_LDBL_MAX, 0);
                    break;
                default:
                    assert(!"invalid type operator -- should never reach here");
                    break;
            }
            break;
        case OP_SUB+OP_I:
            switch(oty->op) {
                case TY_INT:
                    foldbinov(li, -, SI, sub, TG_INT_MIN, TG_INT_MAX, simp_needconst);
                    break;
                case TY_LONG:
                    foldbinov(li, -, SL, sub, TG_LONG_MIN, TG_LONG_MAX, simp_needconst);
                    break;
                default:
                    assert(!"invalid type operator -- should never reach here");
                    break;
            }
            identity(r, l, I, li, 0);
            break;
        case OP_SUB+OP_U:
            switch(oty->op) {
                case TY_UNSIGNED:
                    foldbinnv(-, UI);
                    break;
                case TY_ULONG:
                    foldbinnv(-, UL);
                    break;
                default:
                    assert(!"invalid type operator -- should never reach here");
                    break;
            }
            identity(r, l, U, ul, 0);
            break;
        case OP_SUB+OP_P:
            assert(OP_ISINT(r->op));
            /* adopted from ADD+P */
            if (OP_ISADDR(l->op) && (op_generic(r->op) == OP_ADD || op_generic(r->op) == OP_SUB) &&
                op_generic(r->kid[1]->op) == OP_CNST) {
                assert(OP_ISINT(r->kid[1]->op));
                return simp_tree_s(((op_generic(r->op) == OP_ADD)? OP_SUB: OP_ADD) + TY_POINTER,
                                   ty,
                                   simp_tree_s(OP_SUB + TY_POINTER, ty, l, r->kid[1]), r->kid[0]);
            }
            if (OP_ISADDR(l->op) && op_generic(r->op) == OP_SUB &&
                op_generic(r->kid[0]->op) == OP_CNST) {
                assert(OP_ISINT(r->kid[0]->op));
                return simp_tree_s(OP_ADD + TY_POINTER, ty,
                                   simp_tree_s(OP_SUB + TY_POINTER, ty, l, r->kid[0]), r->kid[1]);
            }
            /* until here */
            if (op_generic(r->op) == OP_CNST) {
                return simp_tree_s(OP_ADD + TY_POINTER, ty, l,
                                   tree_sconst_s((op_optype(r->op) == OP_CNST+OP_I)?
                                                     -r->u.v.li:
                                                     -(long)r->u.v.ul, ty_ptrsinttype));
            }
            break;
        /* MUL */
        case OP_MUL+OP_F:
            switch(oty->op) {
                case TY_FLOAT:
                    foldbinov(f, *, F, mul, -TG_FLT_MAX, TG_FLT_MAX, 0);
                    break;
                case TY_DOUBLE:
                    foldbinov(d, *, D, mul, -TG_DBL_MAX, TG_DBL_MAX, 0);
                    break;
                case TY_LDOUBLE:
                    foldbinov(ld, *, X, mul, -TG_LDBL_MAX, TG_LDBL_MAX, 0);
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
                    foldbinov(li, *, SI, mul, TG_INT_MIN, TG_INT_MAX, simp_needconst);
                    break;
                case TY_LONG:
                    foldbinov(li, *, SL, mul, TG_LONG_MIN, TG_LONG_MAX, simp_needconst);
                    break;
                default:
                    assert(!"invalid type operator -- should never reach here");
                    break;
            }
            commute(l, r);
            identity(l, r, I, li, 1);
            noeffect(l, r, li, 0, tree_sconst_s(0, ty), 0);
            distribute(ADD);
            distribute(SUB);
            stoshift(l, r, LSH);
            break;
        case OP_MUL+OP_U:
            switch(oty->op) {
                case TY_UNSIGNED:
                    foldbinnv(*, UI);
                    break;
                case TY_ULONG:
                    foldbinnv(*, UL);
                    break;
                default:
                    assert(!"invalid type operator -- should never reach here");
                    break;
            }
            commute(l, r);
            identity(l, r, U, ul, 1);
            noeffect(l, r, ul, 0, tree_uconst_s(0, ty), 0);
            distribute(ADD);
            distribute(SUB);
            utoshift(l, r, LSH);
            break;
        /* DIV */
        case OP_DIV+OP_F:
            switch(oty->op) {
                case TY_FLOAT:
                    chkdivby0(f);
                    foldbinov(f, /, F, div, -TG_FLT_MAX, TG_FLT_MAX, 0);
                    break;
                case TY_DOUBLE:
                    chkdivby0(d);
                    foldbinov(d, /, D, div, -TG_DBL_MAX, TG_DBL_MAX, 0);
                    break;
                case TY_LDOUBLE:
                    chkdivby0(ld);
                    foldbinov(ld, /, X, div, -TG_LDBL_MAX, TG_LDBL_MAX, 0);
                    break;
                default:
                    assert(!"invalid type operator -- should never reach here");
                    break;
            }
            break;
        case OP_DIV+OP_I:
            chkdivby0(li);
            switch(oty->op) {
                case TY_INT:
                    foldbinov(li, /, SI, div, TG_INT_MIN, TG_INT_MAX, simp_needconst);
                    break;
                case TY_LONG:
                    foldbinov(li, /, SL, div, TG_LONG_MIN, TG_LONG_MAX, simp_needconst);
                    break;
                default:
                    assert(!"invalid type operator -- should never reach here");
                    break;
            }
            identity(r, l, I, li, 1);
            break;
        case OP_DIV+OP_U:
            chkdivby0(ul);
            switch(oty->op) {
                case TY_UNSIGNED:
                    foldbinnv(/, UI);
                    break;
                case TY_ULONG:
                    foldbinnv(/, UL);
                    break;
                default:
                    assert(!"invalid type operator -- should never reach here");
                    break;
            }
            identity(r, l, U, ul, 1);
            utoshift(r, l, RSH);
            break;
        /* MOD */
        case OP_MOD+OP_I:
            chkdivby0(li);
            switch(oty->op) {
                case TY_INT:
                    foldbinov(li, %, SI, div, TG_INT_MIN, TG_INT_MAX, simp_needconst);
                    break;
                case TY_LONG:
                    foldbinov(li, %, SL, div, TG_LONG_MIN, TG_LONG_MAX, simp_needconst);
                    break;
                default:
                    assert(!"invalid type operator -- should never reach here");
                    break;
            }
            break;
        case OP_MOD+OP_U:
            chkdivby0(ul);
            switch(oty->op) {
                case TY_UNSIGNED:
                    foldbinnv(%, UI);
                    break;
                case TY_ULONG:
                    foldbinnv(%, UL);
                    break;
                default:
                    assert(!"invalid type operator -- should never reach here");
                    break;
            }
            noeffect(r, l, ul, 1, tree_uconst_s(0, ty), 0);
            if (r->op == OP_CNST+sfx && ispow2(r->u.v.ul)) {
                return tree_bit_s(OP_BAND, l, tree_uconst_s(r->u.v.ul-1, oty), ty);
            }
            break;
        /* BAND */
        case OP_BAND+OP_U:
            switch(oty->op) {
                case TY_UNSIGNED:
                    foldbinnv(&, UI);
                    break;
                case TY_ULONG:
                    foldbinnv(&, UL);
                    break;
                default:
                    assert(!"invalid type operator -- should never reach here");
                    break;
            }
            commute(r, l);
            identity(r, l, U, ul, oty->u.sym->u.lim.max.ul);
            noeffect(r, l, ul, 0, tree_uconst_s(0, ty), 0);
            break;
        /* BOR */
        case OP_BOR+OP_U:
            switch(oty->op) {
                case TY_UNSIGNED:
                    foldbinnv(|, UI);
                    break;
                case TY_ULONG:
                    foldbinnv(|, UL);
                    break;
                default:
                    assert(!"invalid type operator -- should never reach here");
                    break;
            }
            commute(r, l);
            identity(r, l, U, ul, 0);
            break;
        /* BXOR */
        case OP_BXOR+OP_U:
            switch(oty->op) {
                case TY_UNSIGNED:
                    foldbinnv(^, UI);
                    break;
                case TY_ULONG:
                    foldbinnv(^, UL);
                    break;
                default:
                    assert(!"invalid type operator -- should never reach here");
                    break;
            }
            commute(r, l);
            identity(r, l, U, ul, 0);
            break;
        /* BCOM */
        case OP_BCOM+OP_I:    /* to be replaced by BCOM+U */
            switch(oty->op) {
                case TY_INT:
                    folduni(tree_sconst_s(SYM_CROPSI(~l->u.v.li), ty));
                    break;
                case TY_LONG:
                    folduni(tree_sconst_s(SYM_CROPSL(~l->u.v.li), ty));
                    break;
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
                    folduni(tree_uconst_s(SYM_CROPUI(~l->u.v.ul), ty));
                    break;
                case TY_ULONG:
                    folduni(tree_uconst_s(SYM_CROPUL(~l->u.v.ul), ty));
                    break;
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
                    folduni(tree_fpconst_s(-l->u.v.f, ty));
                    break;
                case TY_DOUBLE:
                    folduni(tree_fpconst_s(-l->u.v.d, ty));
                    break;
                case TY_LDOUBLE:
                    folduni(tree_fpconst_s(-l->u.v.ld, ty));
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
                    folduni(tree_sconst_s(SYM_CROPSI(-l->u.v.li), ty));
                    break;
                case TY_LONG:
                    folduni(tree_sconst_s(SYM_CROPSL(-l->u.v.li), ty));
                    break;
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
            identity(r, l, I, li, 0);
            break;
        case OP_LSH+OP_U:
            warnovsh(0);
            warnovshcv();
            switch(oty->op) {
                case TY_UNSIGNED:
                    foldshnv(<<, UI);
                    break;
                case TY_ULONG:
                    foldshnv(<<, UL);
                    break;
                default:
                    assert(!"invalid type operator -- should never reach here");
                    break;
            }
            identity(r, l, I, ul, 0);
            break;
        /* RSH */
        case OP_RSH+OP_I:
            warnovsh(0);
            warnovshcv();
            warnnegrsh();
            foldrshov();
            identity(r, l, I, li, 0);
            break;
        case OP_RSH+OP_U:
            warnovsh(0);
            warnovshcv();
            switch(oty->op) {
                case TY_UNSIGNED:
                    foldshnv(>>, UI);
                    break;
                case TY_ULONG:
                    foldshnv(>>, UL);
                    break;
                default:
                    assert(!"invalid type operator -- should never reach here");
                    break;
            }
            identity(r, l, I, ul, 0);
            break;
        /* AND */
        case OP_AND+OP_I:    /* meaningless I */
            op = OP_AND;
            foldlog(!l->u.v.li);
            break;
        /* OR */
        case OP_OR+OP_I:    /* meaningless I */
            op = OP_OR;
            foldlog(l->u.v.li);
            break;
        /* EQ */
        case OP_EQ+OP_F:
            switch(oty->op) {
                case TY_FLOAT:
                    foldcmp(f, ==);
                    break;
                case TY_DOUBLE:
                    foldcmp(d, ==);
                    break;
                case TY_LDOUBLE:
                    foldcmp(ld, ==);
                    break;
                default:
                    assert(!"invalid type operator -- should never reach here");
                    break;
            }
            commute(r, l);
            break;
        case OP_EQ+OP_I:
            foldcmp(li, ==);
            commute(r, l);
            switch(oty->op) {
                case TY_INT:
                    zerofield(EQ, li);
                    break;
                case TY_LONG:
                    zerofieldc(EQ, I, li);
                    break;
                default:
                    assert(!"invalid type operator -- should never reach here");
                    break;
            }
            break;
        case OP_EQ+OP_U:    /* to be replaced by EQ+I */
            foldcmp(ul, ==);
            commute(r, l);
            if (oty->op == TY_UNSIGNED)
                zerofield(EQ, ul);
            zerofieldc(EQ, U, ul);
            symcmpz(0);
            op = OP_EQ+op_sfx(ty_scounter(oty));
            break;
        /* NE */
        case OP_NE+OP_F:
            switch(oty->op) {
                case TY_FLOAT:
                    foldcmp(f, !=);
                    break;
                case TY_DOUBLE:
                    foldcmp(d, !=);
                    break;
                case TY_LDOUBLE:
                    foldcmp(ld, !=);
                    break;
                default:
                    assert(!"invalid type operator -- should never reach here");
                    break;
            }
            break;
        case OP_NE+OP_I:
            foldcmp(li, !=);
            commute(r, l);
            switch(oty->op) {
                case TY_INT:
                    zerofield(NE, li);
                    break;
                case TY_LONG:
                    zerofieldc(NE, I, li);
                    break;
                default:
                    assert(!"invalid type operator -- should never reach here");
                    break;
            }
            break;
        case OP_NE+OP_U:    /* to be replaced by NE+I */
            foldcmp(ul, !=);
            commute(r, l);
            if (oty->op == TY_UNSIGNED)
                zerofield(NE, ul);
            zerofieldc(NE, U, ul);
            symcmpz(1);
            op = OP_NE+op_sfx(ty_scounter(oty));
            break;
        /* GE */
        case OP_GE+OP_F:
            switch(oty->op) {
                case TY_FLOAT:
                    foldcmp(f, >=);
                    break;
                case TY_DOUBLE:
                    foldcmp(d, >=);
                    break;
                case TY_LDOUBLE:
                    foldcmp(ld, >=);
                    break;
                default:
                    assert(!"invalid type operator -- should never reach here");
                    break;
            }
            break;
        case OP_GE+OP_I:
            foldcmp(li, >=);
            break;
        case OP_GE+OP_U:
            foldcmp(ul, >=);
            geu(l, r, 1);
            utoeq(l, EQ);
            break;
        /* GT */
        case OP_GT+OP_F:
            switch(oty->op) {
                case TY_FLOAT:
                    foldcmp(f, >);
                    break;
                case TY_DOUBLE:
                    foldcmp(d, >);
                    break;
                case TY_LDOUBLE:
                    foldcmp(ld, >);
                    break;
                default:
                    assert(!"invalid type operator -- should never reach here");
                    break;
            }
            break;
        case OP_GT+OP_I:
            foldcmp(li, >);
            break;
        case OP_GT+OP_U:
            foldcmp(ul, >);
            geu(r, l, 0);
            utoeq(r, NE);
            break;
        /* LE */
        case OP_LE+OP_F:
            switch(oty->op) {
                case TY_FLOAT:
                    foldcmp(f, <=);
                    break;
                case TY_DOUBLE:
                    foldcmp(d, <=);
                    break;
                case TY_LDOUBLE:
                    foldcmp(ld, <=);
                    break;
                default:
                    assert(!"invalid type operator -- should never reach here");
                    break;
            }
            break;
        case OP_LE+OP_I:
            foldcmp(li, <=);
            break;
        case OP_LE+OP_U:
            foldcmp(ul, <=);
            geu(r, l, 1);
            utoeq(r, EQ);
            break;
        /* LT */
        case OP_LT+OP_F:
            switch(oty->op) {
                case TY_FLOAT:
                    foldcmp(f, <);
                    break;
                case TY_DOUBLE:
                    foldcmp(d, <);
                    break;
                case TY_LDOUBLE:
                    foldcmp(ld, <);
                    break;
                default:
                    assert(!"invalid type operator -- should never reach here");
                    break;
            }
            break;
        case OP_LT+OP_I:
            foldcmp(li, <);
            break;
        case OP_LT+OP_U:
            foldcmp(ul, <);
            geu(l, r, 0);
            utoeq(l, NE);
            break;
        /* NOT */
        case OP_NOT+OP_I:    /* meaningless I */
            op = OP_NOT;
            folduni(tree_sconst_s(!l->u.v.li, ty_inttype));
            break;
        /* OP_POS */
        case OP_POS+OP_F:
        case OP_POS+OP_I:
        case OP_POS+OP_U:
            return l;
        default:
            assert(!"invalid operation code -- should never reach here");
            break;
    }

    return tree_new_s(op, ty, l, r);
}


/*
 *  merges the constant expression info after simplifying a tree
 */
tree_t *(simp_tree_s)(int op, ty_t *ty, tree_t *l, tree_t *r)
{
    tree_t *p = simplify_s(op, ty, l, r);

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
        p->orgn = tree_new_s(op, ty, l, r);
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
static tree_t *cvsimplify_s(int op, ty_t *fty, ty_t *tty, tree_t *l)
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
        case OP_CVI+OP_I:    /* from char/short/int/long */
            samesize();
            if (tty->size > fty->size) {    /* signed char/short to int/long, int to long */
                cvtnv(p->u.v.li = l->u.v.li);
            } else    /* int/long to signed char/short, long to int */
                switch(top) {
                    case TY_CHAR:
                        cvtov(li, TG_SCHAR_MIN, TG_SCHAR_MAX, p->u.v.li = SYM_CROPSC(l->u.v.li));
                        break;
                    case TY_SHORT:
                        cvtov(li, TG_SHRT_MIN, TG_SHRT_MAX, p->u.v.li = SYM_CROPSS(l->u.v.li));
                        break;
                    case TY_INT:
                        cvtov(li, TG_INT_MIN, TG_INT_MAX, p->u.v.li = SYM_CROPSI(l->u.v.li));
                        break;
                    default:
                        assert(!"invalid type operator -- should never reach here");
                        break;
                }
            break;
        case OP_CVU+OP_U:    /* from uint/ulong */
            samesize();
            switch(top) {
                case TY_UNSIGNED:    /* ulong to uint */
                    cvtnv(p->u.v.ul = SYM_CROPUI(l->u.v.ul));
                    break;
                case TY_ULONG:       /* uint to ulong */
                    cvtnv(p->u.v.ul = SYM_CROPUL(l->u.v.ul));
                    break;
                default:
                    assert(!"invalid type operator -- should never reach here");
                    break;
            }
            break;
        case OP_CVF+OP_I:    /* from ldouble */
            switch(top) {
                case TY_INT:    /* to int */
                    cvtov(ld, TG_INT_MIN-1.0L, TG_INT_MAX+1.0L,
                          p->u.v.li = SYM_CROPSI((long)l->u.v.ld));
                    break;
                case TY_LONG:    /* to long */
                    cvtov(ld, TG_LONG_MIN-1.0L, TG_LONG_MAX+1.0L,
                          p->u.v.li = SYM_CROPSL((long)l->u.v.ld));
                    break;
                default:
                    assert(!"invalid type operator -- should never reach here");
                    break;
            }
            break;
        case OP_CVI+OP_F:    /* from int/long */
            /* to ldouble */
            cvtnv(p->u.v.ld = l->u.v.li);
            break;
        case OP_CVI+OP_U:    /* from uchar/ushort/int/long */
            /* to uint/ulong */
            cvtnv(p->u.v.ul = l->u.v.ul);
            break;
        case OP_CVU+OP_I:    /* from uint/ulong */
            switch(top) {
                case TY_CHAR:    /* to uchar */
                    cvtnv(p->u.v.ul = SYM_CROPUC(l->u.v.ul));
                    break;
                case TY_SHORT:    /* to short */
                    cvtnv(p->u.v.ul = SYM_CROPUS(l->u.v.ul));
                    break;
                case TY_INT:      /* to int */
                    cvtus(TG_INT_MAX, p->u.v.li = SYM_CROPSI(l->u.v.ul));
                    break;
                case TY_LONG:    /* to long */
                    cvtus(TG_LONG_MAX, p->u.v.li = SYM_CROPSL(l->u.v.ul));
                    break;
                default:
                    assert(!"invalid type operator -- should never reach here");
                    break;
            }
            break;
        case OP_CVU+OP_P:    /* from uint/ulong */
            cvtnv(p->u.v.tp = l->u.v.ul);
            break;
        case OP_CVP+OP_U:    /* from pointer */
            switch(top) {
                case TY_UNSIGNED:    /* to uint */
                    cvtnv(p->u.v.ul = SYM_CROPUI(l->u.v.tp));
                    break;
                case TY_ULONG:       /* to ulong */
                    cvtnv(p->u.v.ul = SYM_CROPUL(l->u.v.tp));
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

    return tree_new_s(op+OP_CVSFX(fty, tty), tty, l, NULL);
}


/*
 *  merges the constant expression info after simplifying a tree
 */
tree_t *(simp_cvtree_s)(int op, ty_t *fty, ty_t *tty, tree_t *l)
{
    tree_t *p = cvsimplify_s(op, fty, tty, l);

    if (op_generic(p->op) == OP_CNST || op_generic(p->op) == OP_ADDRG)
        p->f.npce |= l->f.npce;

    if (op_generic(p->op) != op_generic(op) || (l && l != l->orgn)) {
        if (l)
            l = l->orgn;
        p->orgn = tree_new_s(op, tty, l, NULL);
    }

    return p;
}

/* end of simp.c */
