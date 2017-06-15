/*
 *  expression tree
 */

#include <stddef.h>        /* NULL */
#include <cbl/assert.h>    /* assert */

#include "lmap.h"
#include "common.h"
#include "err.h"
#include "main.h"
#include "op.h"
#include "simp.h"
#include "tree.h"
#include "ty.h"
#include "enode.h"


/*
 *  transforms a conditional tree to a value tree;
 *  no effect on other trees
 */
tree_t *(enode_value)(tree_t *p)
{
    assert(p);
    assert(ty_inttype);    /* ensures types initialized */

    switch(op_generic(tree_rightkid(p)->op)) {
        case OP_AND:
        case OP_OR:
        case OP_NOT:
        case OP_EQ:
        case OP_NE:
        case OP_LE:
        case OP_LT:
        case OP_GE:
        case OP_GT:
            p = tree_cond(p, tree_sconst(xI, ty_inttype, p->orgn->pos),
                             tree_sconst(xO, ty_inttype, p->orgn->pos),
                          ty_inttype, p->orgn->pos);
            p->orgn = p->orgn->kid[0]->orgn;    /* strips off COND */
            break;
        default:
            break;
    }

    return p;
}


/*
 *  transforms a value tree to a conditional tree;
 *  no effect on other trees;
 *  pointer() applies to the operand
 */
tree_t *(enode_cond)(tree_t *p)
{
    tree_pos_t *tpos;

    assert(p);
    assert(ty_inttype);    /* ensures types initialized */

    switch(op_generic(tree_rightkid(p)->op)) {
        case OP_AND:
        case OP_OR:
        case OP_NOT:
        case OP_EQ:
        case OP_NE:
        case OP_LE:
        case OP_LT:
        case OP_GE:
        case OP_GT:
            return p;
        default:
            break;
    }
    p = enode_pointer(p);
    p = enode_cast(p, ty_ipromote(p->type), 0, NULL);

    tpos = tree_npos1(TREE_TW(p));
    p = tree_cmp(OP_NE, p, tree_sconst(xO, ty_inttype, tpos), NULL, tpos);
    p->orgn = p->orgn->kid[0]->orgn;    /* strips off NE */

    return p;
}


/*
 *  decays an array/function tree to a pointer tree;
 *  no effect on other trees
 */
tree_t *(enode_pointer)(tree_t *p)
{
    tree_t *r;

    assert(p);
    assert(p->type);

    if (TY_ISARRAY(p->type)) {
        assert(p->op != OP_RIGHT || !p->u.sym);
        if (main_opt()->std == 1 && p->op == OP_RIGHT && p->f.nlval) {
            for (r = p; r->kid[1] && r->kid[1]->op == OP_RIGHT &&
                        ty_same(r->kid[1]->type, r->type); r = r->kid[1])
                continue;
            if (!r->f.nlvala) {
                err_dpos(TREE_TW(p), ERR_EXPR_NLVALARR);
                r->f.nlvala = 1;
            }
            return p;
        } else if (OP_ISADDR(p->op) && p->u.sym->f.wregister)
            err_dpos(TREE_TW(p), ERR_EXPR_ATOPREG);
        p = tree_retype(p, ty_atop(p->type), NULL);
    } else if (TY_ISFUNC(p->type))
        p = tree_retype(p, ty_ptr(p->type), NULL);

    return p;
}


/*
 *  applies the usual arithmetic conversion
 */
static ty_t *binary(ty_t *xty, ty_t *yty)
{
    int sx, sy;

    assert(xty);
    assert(yty);

    xty = TY_RMQENUM(xty->t.type);
    yty = TY_RMQENUM(yty->t.type);

    assert(!TY_ISQUAL(xty));
    assert(TY_ISARITH(xty));
    assert(!TY_ISQUAL(yty));
    assert(TY_ISARITH(yty));
    assert(ty_ldoubletype);    /* ensures types initialized */

    if (xty == ty_ldoubletype || yty == ty_ldoubletype)
        return ty_ldoubletype;
    if (xty == ty_doubletype || yty == ty_doubletype)
        return ty_doubletype;
    if (xty == ty_floattype || yty == ty_floattype)
        return ty_floattype;
#ifdef SUPPORT_LL
    if (xty == ty_ullongtype || yty == ty_ullongtype)
        return ty_ullongtype;
#else    /* !SUPPORT_LL */
    if (xty == ty_ulongtype || yty == ty_ulongtype)
        return ty_ulongtype;
#endif    /* SUPPORT_LL */

    xty = ty_ipromote(xty);
    yty = ty_ipromote(yty);
    sx = !TY_ISUNSIGN(xty);
    sy = !TY_ISUNSIGN(yty);

    if (sx == sy)
        return (xty->op > yty->op)? xty: yty;
    if (xgeu(xty->u.sym->u.lim.max.u, yty->u.sym->u.lim.max.u))
        return (sx)? xty: ty_ucounter(yty);
    else
        return (sx)? ty_ucounter(xty): yty;
}


/*
 *  inspects if a tree is a null pointer constant;
 *  ASSUMPTION: NPC of pointer type is represented as 0
 */
int (enode_isnpc)(tree_t *e)
{
    assert(e);
    assert(ty_ulongtype);    /* ensures types initialized */

    return (!(e->f.npce & (TREE_FCOMMA | TREE_FICE)) &&
            ((TY_ISINTEGER(e->type) && op_generic(e->op) == OP_CNST &&
#ifdef SUPPORT_LL
              xe(enode_cast(e, ty_ullongtype, 0, NULL)->u.v.u, xO)) ||
#else    /* !SUPPORT_LL */
              xe(enode_cast(e, ty_ulongtype, 0, NULL)->u.v.u, xO)) ||
#endif    /* SUPPORT_LL */
             (TY_ISPTR(e->type) && TY_UNQUAL(e->type)->type->op == TY_VOID &&
              op_generic(e->op) == OP_CNST && xe(e->u.v.p, xO))));
}


/*
 *  returns the super type of a type;
 *  ASSUMPTION: the super type of pointers is an integer
 */
static ty_t *super(ty_t *ty)
{
    assert(ty);

    ty = TY_RMQENUM(ty->t.type);

    assert(!TY_ISQUAL(ty));
    assert(ty_chartype);    /* ensures types initialized */

    if (TY_ISSCHAR(ty) || ty == ty_shorttype)
        return ty_inttype;
    if (ty == ty_uchartype || ty == ty_chartype || ty == ty_ushorttype)
        return ty_unsignedtype;
    if (TY_ISPTR(ty))
        return ty_ptruinttype;
    if (ty == ty_floattype || ty == ty_doubletype)
        return ty_ldoubletype;

#ifdef SUPPORT_LL
    assert(ty == ty_inttype || ty == ty_unsignedtype || ty == ty_longtype || ty == ty_ulongtype ||
           ty == ty_llongtype || ty == ty_ullongtype || ty == ty_ldoubletype);
#else    /* !SUPPORT_LL */
    assert(ty == ty_inttype || ty == ty_unsignedtype || ty == ty_longtype || ty == ty_ulongtype ||
           ty == ty_ldoubletype);
#endif    /* SUPPORT_LL */

    return ty;
}


/*
 *  checks overflow in advance for conversion;
 *  conversions to consider are:
 *
 *              C S  C S
 *              \ /  \ /
 *    F -      - I    L
 *       = Ld =- U  X Ul
 *    D -      - Ll   Ull
 *
 *  (only to-signed narrowing and fp-to-unsigned conversions need be considered);
 *  ASSUMPTION: TG_FLT_MAX >= TG_{ULONG|ULLONG}_MAX even if inexactly represented
 */
static void chkcvovf(tree_t *p, ty_t *fty, ty_t *tty, int chk, const lmap_t *pos)
{
    ty_t *stty;
    int ovf = 0;

    assert(p);
    assert(!TY_ISQUAL(p->type));
    assert(p->type == p->type->t.type);
    assert(fty);
    assert(tty);
    assert(!TY_ISQUAL(tty));
    assert(ty_ldoubletype);    /* ensures types initialized */

    stty = super(tty);
    if (!chk || op_generic(p->op) != OP_CNST || (p->type != ty_ldoubletype && TY_ISUNSIGN(stty)))
        return;

    switch(p->type->op) {    /* super type of fty */
        case TY_INT:
        case TY_LONG:
#ifdef SUPPORT_LL
        case TY_LLONG:
#endif    /* SUPPORT_LL */
            if (!TY_ISFP(tty) &&
                (xls(p->u.v.s, tty->u.sym->u.lim.min.s) ||
                 xgs(p->u.v.s, tty->u.sym->u.lim.max.s)))
                ovf = 1;
            break;
        case TY_UNSIGNED:
        case TY_ULONG:
#ifdef SUPPORT_LL
        case TY_ULLONG:
#endif    /* SUPPORT_LL */
            if (!TY_ISFP(tty) && xgs(p->u.v.u, tty->u.sym->u.lim.max.s))
                ovf = 1;
            break;
        case TY_LDOUBLE:
            if (tty->t.type == ty_floattype &&    /* X to F */
                (p->u.v.ld < -tty->u.sym->u.lim.max.f || p->u.v.ld > tty->u.sym->u.lim.max.f))
                ovf = 1;
            else if (tty->t.type == ty_doubletype &&    /* X to D */
                     (p->u.v.ld < -tty->u.sym->u.lim.max.d || p->u.v.ld > tty->u.sym->u.lim.max.d))
                ovf = 1;
            else if (TY_ISUNSIGN(stty)) {    /* X to unsigned */
                if (p->u.v.ld <= -1.0L || p->u.v.ld >= xcuf(tty->u.sym->u.lim.max.u) + 1.0L)
                    ovf = 1;
            } else if (TY_ISINTEGER(tty)) {    /* X to signed */
                ty_t *ty = TY_RMQENUM(tty);
                if (p->u.v.ld <= xcsf(ty->u.sym->u.lim.min.s) - 1.0L ||
                    p->u.v.ld >= xcsf(ty->u.sym->u.lim.max.s) + 1.0L)
                    ovf = 1;
            }
            break;
        default:
            assert(!"invalid type operator -- should never reach here");
            break;
    }
    if (ovf) {
        if (!pos)
            pos = TREE_TO(p);    /* to make non-null */
        err_dmpos(TREE_TW(p), ERR_EXPR_OVFCONV, pos, NULL, fty, tty);
    }
}


/*
 *  converts a type to another type;
 *  supported conversions are:
 *
 *              C S  C S
 *              \ /  \ /
 *    F -      - I    L
 *       = Ld =- U  X Ul  - P
 *    D -      - Ll   Ull
 *
 *  (only one of U-P or Ul-P is used depending on the target);
 *  ASSUMPTION: enum is always int;
 *  ASSUMPTION: all pointers are uniform (same representation);
 *  ASSUMPTION: max value of unsigned >> 1 == max value of signed;
 *  TODO: improve conversions related to fp types
 */
tree_t *(enode_cast)(tree_t *p, ty_t *tty, int chkovf, const lmap_t *pos)
{
    ty_t *fty, *stty, *oty;

    assert(p);
    assert(enode_value(p) == p);
    assert(p->type);
    assert(tty);
    assert(ty_chartype);    /* ensures types initialized */

    fty = TY_UNQUAL(p->type);
    tty = TY_UNQUAL(tty);
    if (ty_same((oty=TY_RMQENUM(fty->t.type)), TY_RMQENUM(tty->t.type)))
        return tree_retype(p, tty, NULL);    /* retains typedef */

    if (TY_ISSCHAR(oty) || oty == ty_shorttype)
        p = simp_cvtree(OP_CVI, oty, super(oty), p);
    else if (oty == ty_uchartype || oty == ty_chartype || oty == ty_ushorttype)
        p = simp_cvtree(OP_CVI, oty, super(oty), p);
    else if (TY_ISPTR(oty)) {
        if (TY_ISPTR(tty)) {
            if ((TY_ISFUNC(oty->type) && !TY_ISFUNC(tty->type)) ||
                (TY_ISFUNC(tty->type) && !enode_isnpc(p) && !TY_ISFUNC(oty->type)))
                err_dmpos(pos, ERR_EXPR_FPTROPTR, TREE_TW(p), NULL);
            return tree_retype(p, tty, NULL);
        } else
            p = simp_cvtree(OP_CVP, oty, ty_ptruinttype, p);
    } else if (oty == ty_floattype || oty == ty_doubletype)
        p = simp_cvtree(OP_CVF, oty, super(oty), p);
    else
        p = tree_retype(p, oty, NULL);
    {    /* ldouble, int, uint, long, ulong, llong and ullong remained for fty */
        static int cvtab[][13] = {    /* [from][to] */
            /*         0  F  D    Ld    C  S    I     e    U       L       Ul      Ll      Ull */
            /* 0   */  0, 0, 0,      0, 0, 0,      0, 0,      0,      0,      0,      0,      0,
            /* F   */  0, 0, 0,      0, 0, 0,      0, 0,      0,      0,      0,      0,      0,
            /* D   */  0, 0, 0,      0, 0, 0,      0, 0,      0,      0,      0,      0,      0,
            /* Ld  */  0, 0, 0,      0, 0, 0, OP_CVF, 0,     -2, OP_CVF,     -2, OP_CVF,     -2,
            /* C   */  0, 0, 0,      0, 0, 0,      0, 0,      0,      0,      0,      0,      0,
            /* S   */  0, 0, 0,      0, 0, 0,      0, 0,      0,      0,      0,      0,      0,
            /* I   */  0, 0, 0, OP_CVI, 0, 0,      0, 0, OP_CVI, OP_CVI, OP_CVI, OP_CVI, OP_CVI,
            /* e   */  0, 0, 0,      0, 0, 0,      0, 0,      0,      0,      0,      0,      0,
            /* U   */  0, 0, 0,     -1, 0, 0, OP_CVU, 0,      0, OP_CVU, OP_CVU, OP_CVU, OP_CVU,
            /* L   */  0, 0, 0, OP_CVI, 0, 0, OP_CVI, 0, OP_CVI,      0, OP_CVI, OP_CVI, OP_CVI,
            /* Ul  */  0, 0, 0,     -1, 0, 0, OP_CVU, 0, OP_CVU, OP_CVU,      0, OP_CVU, OP_CVU,
            /* Ll  */  0, 0, 0, OP_CVI, 0, 0, OP_CVI, 0, OP_CVI, OP_CVI, OP_CVI,      0, OP_CVI,
            /* Ull */  0, 0, 0,     -1, 0, 0, OP_CVU, 0, OP_CVU, OP_CVU, OP_CVU, OP_CVU,      0,
        };

        ty_t *sty;
        tree_t *c;
        int npce, op;
        tree_pos_t *tpos = tree_npos1(TREE_TW(p));

        /* after p got super type and before fty changes */
        chkcvovf(p, fty, tty, chkovf, pos);

        stty = super(tty);
        fty = p->type;
        if (fty != stty) {
            err_mute();    /* done in chkcvovf() */
            op = cvtab[fty->op][stty->op];
            switch(op) {
                case -1:    /* uint/ulong/ullong to ldouble */
                    npce = p->f.npce;
                    sty = ty_scounter(fty);
                    c = tree_fpconst(2.0, ty_ldoubletype, tpos);
                    p = tree_optree['+'](OP_ADD,
                            tree_optree['*'](OP_MUL,
                                c,
                                simp_cvtree(OP_CVU, fty, sty,
                                    tree_sh(OP_RSH, p, tree_sconst(xI, sty, tpos), fty, tpos)),
                                NULL, tpos),
                            simp_cvtree(OP_CVU, fty, sty,
                                tree_bit(OP_BAND, p, tree_sconst(xI, sty, tpos), fty, tpos)),
                            NULL, tpos);
                    p->f.npce = npce;
                    break;
                case -2:    /* ldouble to uint/ulong/ullong */
                    npce = p->f.npce;
                    sty = ty_scounter(stty);
                    c = tree_fpconst(1.0L+xcuf(sty->u.sym->u.lim.max.u), ty_ldoubletype, tpos);
                    p = tree_cond(
                            tree_cmp(OP_GE, p, c, ty_ldoubletype, tpos),
                            tree_optree['+'](OP_ADD,
                                enode_cast(
                                    enode_cast(tree_sub(OP_SUB, p, c, ty_ldoubletype, tpos),
                                               sty, chkovf, pos),
                                    stty, chkovf, pos),
                                tree_uconst(xau(xctu(sty->u.sym->u.lim.max.s), xI), stty, tpos),
                                NULL, tpos),
                            simp_cvtree(OP_CVF, fty, sty, p),
                            NULL, tpos);
                    p->f.cvfpu = 1;
                    p->f.npce = npce;
                    break;
                case 0:
                    assert(!"invalid conversion operator -- should never reach here");
                    break;
                default:
                    p = simp_cvtree(op, fty, stty, p);
                    break;
            }
            err_unmute();
        }
    }
    oty = TY_RMQENUM(tty->t.type);
    if (TY_ISSCHAR(oty) || oty == ty_shorttype)
        p = simp_cvtree(OP_CVI, stty, tty, p);
    else if (oty == ty_uchartype || oty == ty_chartype || oty == ty_ushorttype)
        p = simp_cvtree(OP_CVU, stty, tty, p);
    else if (TY_ISPTR(oty))
        p = simp_cvtree(OP_CVU, stty, tty, p);
    else if (oty == ty_floattype || oty == ty_doubletype)
        p = simp_cvtree(OP_CVF, stty, tty, p);
    else
        p = tree_retype(p, tty, NULL);

    return p;
}


#define LEFT_HAS_RIGHTS_QUAL(l, r) ((l->op & r->op & TY_CONSVOL) == (r->op & TY_CONSVOL))

/*
 *  checks types for the = operator (type-tree version);
 *  void signals assignment of compatible incomplete types;
 *  ASSUMPTION: enum is always int
 */
ty_t *(enode_tcasgnty)(ty_t *ty, tree_t *r, const lmap_t *pos, tree_t *l)
{
    int ret;
    ty_t *lty, *rty;

    assert(ty);
    assert(r);
    assert(r->type);
    assert(pos);
    assert(ty_voidtype);    /* ensures types initialized */

    lty = TY_UNQUAL(ty);
    rty = TY_UNQUAL(r->type);

    if ((TY_ISARITH(lty) && TY_ISARITH(rty)) ||
        ((TY_ISSTRUNI(lty) || lty->t.type == ty_voidtype) && lty->t.type == rty->t.type))
        return (lty->size == 0 || rty->size == 0)? ty_voidtype: lty;
    if (lty->size == 0 || rty->size == 0)
        return NULL;
    if (TY_ISPTR(lty) && enode_isnpc(r))
        return lty;
    if (((TY_ISVOIDP(lty) && TY_ISPTR(rty)) || (TY_ISPTR(lty) && TY_ISVOIDP(rty))) &&
        LEFT_HAS_RIGHTS_QUAL(lty->type, rty->type))
        return lty;
    if (TY_ISPTR(lty) && TY_ISPTR(rty) &&
        (ret = ty_equiv(TY_UNQUAL(lty->type), TY_UNQUAL(rty->type), 1)) != 0 &&
        LEFT_HAS_RIGHTS_QUAL(lty->type, rty->type)) {
        if (ret > 1) {
            if (!l)
                l = r;    /* to make non-null */
            err_dmpos(pos, ERR_EXPR_ASGNENUMPTR, TREE_TW(l), TREE_TW(r), NULL, ty, rty);
        }

        return lty;
    }

    return NULL;
}


/*
 *  checks types for the = operator (tree-tree version);
 */
ty_t *(enode_tcasgn)(tree_t *l, tree_t *r, tree_pos_t *tpos)
{
    assert(l);

    return enode_tcasgnty(l->type, r, TREE_PO(tpos), l);
}


/*
 *  checks types for the ?: operator;
 *  the type of the first operand should be checked elsewhere
 */
ty_t *(enode_tccond)(tree_t *l, tree_t *r)
{
    ty_t *lty, *rty, *ty;

    assert(l);
    assert(l->type);
    assert(r);
    assert(r->type);
    assert(ty_voidptype);    /* ensures types initialized */

    lty = TY_UNQUAL(l->type);
    rty = TY_UNQUAL(r->type);

    if (TY_ISARITH(lty) && TY_ISARITH(rty))
        ty = binary(lty, rty);
    else if (ty_equiv(lty, rty, 1))
        ty = ty_compose(lty, rty);
    else if (TY_ISPTR(lty) && enode_isnpc(r))
        ty = lty;
    else if (enode_isnpc(l) && TY_ISPTR(rty))
        ty = rty;
    else if ((TY_ISPTR(lty) && !TY_ISFUNC(lty->type) && TY_ISVOIDP(rty)) ||
             (TY_ISPTR(rty) && !TY_ISFUNC(rty->type) && TY_ISVOIDP(lty)))
        ty = ty_voidptype;
    else if (TY_ISPTR(lty) && TY_ISPTR(rty) &&
             ty_equiv(TY_UNQUAL(lty->type), TY_UNQUAL(rty->type), 1))
        ty = ty_ptr(ty_compose(TY_UNQUAL(lty->type), TY_UNQUAL(rty->type)));
    else
        ty = NULL;

    if (ty && TY_ISPTR(ty)) {
        ty = ty_ptr(ty_qualc(((TY_ISPTR(lty))? lty->type->op & TY_CONSVOL: 0) |
                             ((TY_ISPTR(rty))? rty->type->op & TY_CONSVOL: 0), ty->type));
        if (lty == rty && lty->t.type == ty)    /* retains typedef */
            ty = lty;
    }

    return ty;
}


/*
 *  checks types for the && and || operators;
 *  tree_t pointer could be const-qualified due to simplicity, but not for consistency
 */
ty_t *(enode_tcand)(tree_t *l, tree_t *r)
{
    assert(l);
    assert(l->type);
    assert(r);
    assert(r->type);
    assert(ty_inttype);    /* ensures types initialized */

    if (TY_ISSCALAR(l->type) && TY_ISSCALAR(r->type))
        return ty_inttype;

    return NULL;
}


/*
 *  checks types for the &, |, ^ and % operators
 */
ty_t *(enode_tcbit)(tree_t *l, tree_t *r)
{
    ty_t *lty, *rty;

    assert(l);
    assert(l->type);
    assert(r);
    assert(r->type);

    lty = TY_UNQUAL(l->type);
    rty = TY_UNQUAL(r->type);

    if (TY_ISINTEGER(lty) && TY_ISINTEGER(rty))
        return binary(lty, rty);

    return NULL;
}


/*
 *  checks types for the == and != operators;
 *  void signals swapping operands is necessary;
 *  void pointer signals operands are pointers;
 *  note enode_tccmp() called otherwise
 */
ty_t *(enode_tceq)(tree_t *l, tree_t *r)
{
    ty_t *lty, *rty;

    assert(l);
    assert(l->type);
    assert(r);
    assert(r->type);
    assert(ty_voidtype);    /* ensures types initialized */

    lty = TY_UNQUAL(l->type);
    rty = TY_UNQUAL(r->type);

    if ((TY_ISPTR(lty) && enode_isnpc(r)) ||
        (TY_ISPTR(lty) && !TY_ISFUNC(lty->type) && TY_ISVOIDP(rty)) ||
        (TY_ISPTR(lty) && TY_ISPTR(rty) && ty_equiv(lty, rty, 1)))
        return ty_voidptype;
    else if ((enode_isnpc(l) && TY_ISPTR(rty)) ||
             (TY_ISVOIDP(lty) && TY_ISPTR(rty) && !TY_ISFUNC(rty->type)))
        return ty_voidtype;
    else
        return enode_tccmp(l, r);
}


/*
 *  checks if two pointers are compatible for relational comparisons
 */
static int compatible(ty_t *lty, ty_t *rty)
{
    assert(lty);
    assert(!TY_ISQUAL(lty));
    assert(rty);
    assert(!TY_ISQUAL(rty));

    return (TY_ISPTR(lty) && !TY_ISFUNC(lty->type) && TY_ISPTR(rty) && !TY_ISFUNC(rty->type) &&
            ty_equiv(TY_UNQUAL(lty->type), TY_UNQUAL(rty->type), 0));
}


/*
 *  checks types for the <, <=, > and >= operators;
 *  void pointer signals operands are pointers;
 *  note enode_tccmp() used by enode_tceq()
 */
ty_t *(enode_tccmp)(tree_t *l, tree_t *r)
{
    ty_t *lty, *rty;

    assert(l);
    assert(l->type);
    assert(r);
    assert(r->type);
    assert(ty_voidptype);    /* ensures types initialized */

    lty = TY_UNQUAL(l->type);
    rty = TY_UNQUAL(r->type);

    if (TY_ISARITH(lty) && TY_ISARITH(rty))
        return binary(lty, rty);
    else if (compatible(lty, rty))
        return ty_voidptype;

    return NULL;
}

/*
 *  checks types for the << and >> operator
 */
ty_t *(enode_tcsh)(tree_t *l, tree_t *r)
{
    ty_t *lty;

    assert(l);
    assert(l->type);
    assert(r);
    assert(r->type);

    lty = TY_UNQUAL(l->type);

    if (TY_ISINTEGER(lty) && TY_ISINTEGER(r->type))
        return ty_ipromote(lty);

    return NULL;
}


/*
 *  checks types for the binary + operator;
 *  void signals that swapping operands is necessary
 */
ty_t *(enode_tcadd)(tree_t *l, tree_t *r)
{
    ty_t *lty, *rty;

    assert(l);
    assert(l->type);
    assert(r);
    assert(r->type);
    assert(ty_voidtype);    /* ensures types initialized */

    lty = TY_UNQUAL(l->type);
    rty = TY_UNQUAL(r->type);

    if (TY_ISARITH(lty) && TY_ISARITH(rty))
        return binary(lty, rty);
    else if (TY_ISPTR(lty) && TY_ISINTEGER(rty))
        return ty_voidtype;
    else if (TY_ISINTEGER(lty) && TY_ISPTR(rty) && !TY_ISFUNC(rty->type))
        return rty;

    return NULL;
}


/*
 *  checks types for the binary - operator;
 *  pointer signals an integer is subtracted from a pointer;
 *  void signals operands are pointers
 */
ty_t *(enode_tcsub)(tree_t *l, tree_t *r)
{
    ty_t *lty, *rty;

    assert(l);
    assert(l->type);
    assert(r);
    assert(r->type);
    assert(ty_voidtype);    /* ensures types initialized */

    lty = TY_UNQUAL(l->type);
    rty = TY_UNQUAL(r->type);

    if (TY_ISARITH(lty) && TY_ISARITH(rty))
        return binary(lty, rty);
    else if (TY_ISPTR(lty) && !TY_ISFUNC(lty->type) && TY_ISINTEGER(rty))
        return lty;
    else if (compatible(lty, rty))
        return ty_voidtype;

    return NULL;
}


/*
 *  checks types for binary * and / operators
 */
ty_t *(enode_tcmul)(tree_t *l, tree_t *r)
{
    ty_t *lty, *rty;

    assert(l);
    assert(l->type);
    assert(r);
    assert(r->type);

    lty = TY_UNQUAL(l->type);
    rty = TY_UNQUAL(r->type);

    if (TY_ISARITH(lty) && TY_ISARITH(rty))
        return binary(lty, rty);

    return NULL;
}


/*
 *  checks types for the unary + and - operators
 */
ty_t *(enode_tcposneg)(tree_t *p)
{
    assert(p);
    assert(p->type);

    return ((TY_ISARITH(p->type))? ty_ipromote(p->type): NULL);
}


/*
 *  checks types for the ~ operator
 */
ty_t *(enode_tcbcom)(tree_t *p)
{
    assert(p);
    assert(p->type);

    return ((TY_ISINTEGER(p->type))? ty_ipromote(p->type): NULL);
}


/*
 *  checks types for the ! operator;
 *  tree_t pointer could be const-qualified due to simplicity, but not for consistency
 */
ty_t *(enode_tcnot)(tree_t *p)
{
    assert(p);
    assert(p->type);
    assert(ty_inttype);    /* ensures types initialized */

    return ((TY_ISSCALAR(p->type))? ty_inttype: NULL);
}


/*
 *  checks types for the unary * operator
 */
ty_t *(enode_tcindir)(tree_t *p, tree_pos_t *tpos)
{
    ty_t *ty;

    assert(p);
    assert(p->type);
    assert(tpos);

    ty = TY_UNQUAL(p->type);

    if (TY_ISPTR(ty) && (TY_ISFUNC(ty->type) || TY_ISARRAY(ty->type)))
        return ty->type;
    else {
        ty = ty_deref(ty);
        if (!ty)
            err_dtpos(tpos, p, NULL, ERR_EXPR_POINTER, p->type);
        return ty;
    }
}


/*
 *  checks types for the unary & operator;
 *  void signals &*(ptr to void);
 *  const void signals &(void symbol)
 */
ty_t *(enode_tcaddr)(tree_t *p)
{
    assert(p);
    assert(p->type);
    assert(ty_voidptype);    /* ensures types initialized */

    if ((TY_ISFUNC(p->type) || TY_ISARRAY(p->type)) && op_generic(p->op) != OP_RIGHT)
        return ty_ptr(p->type);

    if (op_generic(p->op) != OP_INDIR)
        return NULL;
    else if (ty_same(p->kid[0]->type, ty_voidptype)) {
        if (p->f.eindir)    /* &*(ptr to void) */
            return ty_voidtype;
        else                /* &(void symbol) */
            return ty_qual(TY_CONST, ty_voidtype, 0, NULL);    /* always succeeds */
    }

    return p->kid[0]->type;
}


/*
 *  checks if a conditional has an appropriate type
 */
tree_t *(enode_chkcond)(int op, tree_t *p, const char *name)
{
    static const struct {
        int op;
        const char *name;
    } optab[] = { OP_COND, "?:", OP_AND, "&&", OP_OR, "||", 0, NULL };

    int i;

    assert(p);

    if (op) {
        op = op_generic(op);
        for (i = 0; optab[i].op; i++)
            if (op == optab[i].op)
                break;
    }
    assert(!op || optab[i].name);

    p = enode_pointer(p);
    if (!TY_ISSCALAR(p->type)) {
        err_dpos(TREE_TW(p), ERR_EXPR_CONDTYPE,
                 (!op)? "conditional": name, (!op)? "": optab[i].name, p->type);
        p = NULL;
    }

    return p;
}


/*
 *  prints a type error for an operator
 */
tree_t *(enode_tyerr)(int op, tree_t *l, tree_t *r, tree_pos_t *tpos)
{
    static struct {
        int op;
        const char *name;
    } optab[] = {
        OP_ASGN, "=",  OP_INDIR, "*",   OP_NEG,   "-",  OP_CALL,  "()", OP_ADD,   "+",
        OP_SUB,  "-",  OP_LSH,   "<<",  OP_MOD,   "%",  OP_RSH,   ">>", OP_BAND,  "&",
        OP_BCOM, "~",  OP_BOR,   "|",   OP_BXOR,  "^",  OP_DIV,   "/",  OP_MUL,   "*",
        OP_EQ,   "==", OP_GE,    ">=",  OP_GT,    ">",  OP_LE,    "<=", OP_LT,    "<",
        OP_NE,   "!=", OP_NOT,   "!",   OP_COND,  "?:", OP_RIGHT, ",",  OP_INCR,  "++",
        OP_DECR, "--", OP_SUBS,  "[]",  OP_CADD,  "+=", OP_CSUB,  "-=", OP_CLSH,  "<<=",
        OP_CMOD, "%=", OP_CRSH,  ">>=", OP_CBAND, "&=", OP_CBOR,  "|=", OP_CBXOR, "^=",
        OP_CDIV, "/=", OP_CMUL,  "*=",
        0,        NULL    /* checks for OP_AND and OP_OR done in enode_chkcond() */
    };

    int i;

    assert(l);
    assert(tpos);

    op = op_generic(op);
    for (i = 0; optab[i].op; i++)
        if (op == optab[i].op)
            break;
    assert(optab[i].name);

    if (r) {
        if ((TY_ISSTRUNI(l->type) || TY_ISVOID(l->type)) &&
            ty_equiv(TY_UNQUAL(l->type), TY_UNQUAL(r->type), 1))
            err_dtpos(tpos, l, r, ERR_EXPR_ILLTYPE, l->type, optab[i].name);
        else
            err_dtpos(tpos, l, r, ERR_EXPR_BINOPERR, optab[i].name, l->type, r->type);
    } else
        err_dtpos(tpos, l, NULL, ERR_EXPR_UNIOPERR, optab[i].name, l->type);

    return NULL;
}

/* end of enode.c */
