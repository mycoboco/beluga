/*
 *  expression tree
 */

#include <stddef.h>        /* NULL */
#include <cbl/assert.h>    /* assert */

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
tree_t *(enode_value_s)(tree_t *p)
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
            p = tree_cond_s(p, tree_sconst_s(1, ty_inttype), tree_sconst_s(0, ty_inttype),
                            ty_inttype);
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
tree_t *(enode_cond_s)(tree_t *p)
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
            return p;
        default:
            break;
    }
    p = enode_pointer_s(p);
    p = enode_cast_s(p, ty_ipromote(p->type), 0);

    return tree_cmp_s(OP_NE, p, tree_sconst_s(0, ty_inttype), NULL);
}


/*
 *  decays an array/function tree to a pointer tree;
 *  no effect on other trees
 */
tree_t *(enode_pointer_s)(tree_t *p)
{
    tree_t *r;

    assert(p);
    assert(p->type);

    if (TY_ISARRAY(p->type)) {
        assert(p->op != OP_RIGHT || !p->u.sym);
        if (main_opt()->std == 1 && p->op == OP_RIGHT) {
            for (r = p; r->kid[1] && r->kid[1]->op == OP_RIGHT &&
                        ty_same(r->kid[1]->type, r->type); r = r->kid[1])
                continue;
            if (!r->f.nlvala) {
                err_issuep(&p->pos, ERR_EXPR_NLVALARR);
                r->f.nlvala = 1;
            }
            return p;
        } else if (OP_ISADDR(p->op) && p->u.sym->f.wregister)
            err_issuep(&p->pos, ERR_EXPR_ATOPREG);
        p = tree_retype_s(p, ty_atop_s(p->type));
    } else if (TY_ISFUNC(p->type))
        p = tree_retype_s(p, ty_ptr(p->type));

    return p;
}


/*
 *  applies the usual arithmetic conversion
 */
static ty_t *binary(ty_t *xty, ty_t *yty)
{
    static ty_t **tab[2][3][3] = {
        { /* long >= unsigned */
                          /* long          unsigned          int */
          /* long */     { &ty_longtype, &ty_longtype,     &ty_longtype },
          /* unsigned */ { &ty_longtype, &ty_unsignedtype, &ty_unsignedtype },
          /* int */      { &ty_longtype, &ty_unsignedtype, &ty_inttype },
        },
        { /* long < unsigned */
                          /* long           unsigned          int */
          /* long */     { &ty_longtype,  &ty_ulongtype,    &ty_longtype },
          /* unsigned */ { &ty_ulongtype, &ty_unsignedtype, &ty_unsignedtype },
          /* int */      { &ty_longtype,  &ty_unsignedtype, &ty_inttype }
        }
    };

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
    if (xty == ty_ulongtype || yty == ty_ulongtype)
        return ty_ulongtype;

    xty = ty_ipromote(xty);
    yty = ty_ipromote(yty);

    return *tab[(TG_LONG_MAX >= TG_UINT_MAX)? 0: 1]
               [(xty == ty_longtype)? 0: (xty == ty_unsignedtype)? 1: 2]
               [(yty == ty_longtype)? 0: (yty == ty_unsignedtype)? 1: 2];
}


/*
 *  inspects if a tree is a null pointer constant;
 *  ASSUMPTION: NPC of pointer type is represented as 0
 */
int (enode_isnpc_s)(tree_t *e)
{
    assert(e);
    assert(ty_ulongtype);    /* ensures types initialized */

    return (!(e->f.npce & (TREE_FCOMMA | TREE_FICE)) &&
            ((TY_ISINTEGER(e->type) && op_generic(e->op) == OP_CNST &&
              enode_cast_s(e, ty_ulongtype, 0)->u.v.ul == 0) ||
             (TY_ISPTR(e->type) && TY_UNQUAL(e->type)->type->op == TY_VOID &&
              op_generic(e->op) == OP_CNST && e->u.v.tp == 0)));
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

    assert(ty == ty_inttype || ty == ty_unsignedtype || ty == ty_longtype || ty == ty_ulongtype ||
           ty == ty_ldoubletype);

    return ty;
}


/*
 *  checks overflow in advance for conversion;
 *  conversions to consider are:
 *
 *             C S
 *             \ /
 *    F -     - I - U
 *       = X =  | X
 *    D -     - L - M
 *             / \
 *             C S
 *
 *  (only to-signed narrowing and fp-to-unsigned conversions need be considered);
 *  ASSUMPTION: TG_FLT_MAX is not smaller than TG_ULONG_MAX even if inexactly represented
 */
static void chkcvovf_s(tree_t *p, ty_t *fty, ty_t *tty, int f)
{
    ty_t *stty;
    int ovf = 0;

    assert(p);
    assert(!TY_ISQUAL(p->type));
    assert(p->type == p->type->t.type);
    assert(tty);
    assert(!TY_ISQUAL(tty));
    assert(ty_ldoubletype);    /* ensures types initialized */

    stty = super(tty);
    if (f != ENODE_FCHKOVF || op_generic(p->op) != OP_CNST ||
        (p->type != ty_ldoubletype && TY_ISUNSIGN(stty)))
        return;

    switch(p->type->op) {    /* super type of fty */
        case TY_INT:
        case TY_LONG:
            if (!TY_ISFP(tty) &&
                (p->u.v.li < tty->u.sym->u.lim.min.li ||
                 p->u.v.li > tty->u.sym->u.lim.max.li))
                ovf = 1;
            break;
        case TY_UNSIGNED:
        case TY_ULONG:
            if (!TY_ISFP(tty) && p->u.v.ul > tty->u.sym->u.lim.max.li)
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
                if (p->u.v.ld <= -1.0L || p->u.v.ld >= tty->u.sym->u.lim.max.ul + 1.0L)
                    ovf = 1;
            } else if (TY_ISINTEGER(tty)) {    /* X to signed */
                ty_t *ty = TY_RMQENUM(tty);
                if (p->u.v.ld <= ty->u.sym->u.lim.min.li - 1.0L ||
                    p->u.v.ld >= ty->u.sym->u.lim.max.li + 1.0L)
                    ovf = 1;
            }
            break;
        default:
            assert(!"invalid type operator -- should never reach here");
            break;
    }
    if (ovf)
        err_issue_s(ERR_EXPR_OVFCONV, fty, tty);
}


/*
 *  converts a type to another type;
 *  supported conversions are:
 *
 *             C S C S
 *             \ / \ /
 *    F -     - I - U -
 *       = X =  | X |  = P
 *    D -     - L - M -
 *             / \ / \
 *             C S C S
 *
 *  (only one of U-P or M-P is used depending on the target);
 *  ASSUMPTION: enum is always int;
 *  ASSUMPTION: all pointers are uniform (same representation);
 *  ASSUMPTION: max value of unsigned >> 1 == max value of signed;
 *  ASSUMPTION: long double can represent TG_{INT,LONG}_MAX+1 correctly;
 *  TODO: improve conversions related to fp types
 */
tree_t *(enode_cast_s)(tree_t *p, ty_t *tty, int f)
{
    ty_t *fty, *stty, *oty;

    assert(p);
    assert(enode_value_s(p) == p);
    assert(p->type);
    assert(tty);
    assert(ty_chartype);    /* ensures types initialized */

    fty = TY_UNQUAL(p->type);
    tty = TY_UNQUAL(tty);
    if (ty_same((oty=TY_RMQENUM(fty->t.type)), TY_RMQENUM(tty->t.type))) {
        p = tree_retype_s(p, tty);    /* sets pos and retains typedef */
        goto ret;
    }

    if (TY_ISSCHAR(oty) || oty == ty_shorttype)
        p = simp_cvtree_s(OP_CVI, oty, super(oty), p);
    else if (oty == ty_uchartype || oty == ty_chartype || oty == ty_ushorttype)
        p = simp_cvtree_s(OP_CVI, oty, super(oty), p);
    else if (TY_ISPTR(oty)) {
        if (TY_ISPTR(tty)) {
            if ((TY_ISFUNC(oty->type) && !TY_ISFUNC(tty->type)) ||
                (TY_ISFUNC(tty->type) && !enode_isnpc_s(p) && !TY_ISFUNC(oty->type)))
                err_issue_s(ERR_EXPR_FPTROPTR);
            p = tree_retype_s(p, tty);
            goto ret;
        } else
            p = simp_cvtree_s(OP_CVP, oty, ty_ptruinttype, p);
    } else if (oty == ty_floattype || oty == ty_doubletype)
        p = simp_cvtree_s(OP_CVF, oty, super(oty), p);
    else
        p = tree_retype_s(p, oty);
    {    /* ldouble, int, uint, long and ulong remained for fty */
        static int cvtab[][11] = {    /* [from][to] */
            /*       0  F  D    X     C  S    I     e   U       L       M     */
            /* 0 */  0, 0, 0,      0, 0, 0,      0, 0,      0,      0,      0,
            /* F */  0, 0, 0,      0, 0, 0,      0, 0,      0,      0,      0,
            /* D */  0, 0, 0,      0, 0, 0,      0, 0,      0,      0,      0,
            /* X */  0, 0, 0,      0, 0, 0, OP_CVF, 0,     -2, OP_CVF,     -2,
            /* C */  0, 0, 0,      0, 0, 0,      0, 0,      0,      0,      0,
            /* S */  0, 0, 0,      0, 0, 0,      0, 0,      0,      0,      0,
            /* I */  0, 0, 0, OP_CVI, 0, 0, OP_CVI, 0, OP_CVI, OP_CVI, OP_CVI,
            /* e */  0, 0, 0,      0, 0, 0,      0, 0,      0,      0,      0,
            /* U */  0, 0, 0,     -1, 0, 0, OP_CVU, 0, OP_CVU, OP_CVU, OP_CVU,
            /* L */  0, 0, 0, OP_CVI, 0, 0, OP_CVI, 0, OP_CVI, OP_CVI, OP_CVI,
            /* M */  0, 0, 0,     -1, 0, 0, OP_CVU, 0, OP_CVU, OP_CVU, OP_CVU
        };

        ty_t *sty;
        tree_t *c;
        int npce, op;

        chkcvovf_s(p, fty, tty, f & (ENODE_FECAST|ENODE_FCHKOVF));    /* before fty changes and
                                                                         after p got super type */
        stty = super(tty);
        fty = p->type;
        if (fty != stty) {
            err_mute();    /* done in chkcvovf_s() */
            op = cvtab[fty->op][stty->op];
            switch(op) {
                case -1:    /* uint/ulong to ldouble */
                    npce = p->f.npce;
                    sty = ty_scounter(fty);
                    c = tree_fpconst_s(2.0, ty_ldoubletype);
                    p = tree_optree_s['+'](OP_ADD,
                            tree_optree_s['*'](OP_MUL,
                                c,
                                simp_cvtree_s(OP_CVU, fty, sty,
                                    tree_sh_s(OP_RSH, p, tree_sconst_s(1, sty), fty)),
                                NULL),
                            simp_cvtree_s(OP_CVU, fty, sty,
                                tree_bit_s(OP_BAND, p, tree_sconst_s(1, sty), fty)),
                            NULL);
                    p->f.npce = npce;
                    break;
                case -2:    /* ldouble to uint/ulong */
                    npce = p->f.npce;
                    sty = ty_scounter(stty);
                    c = tree_fpconst_s(1.0L+sty->u.sym->u.lim.max.ul, ty_ldoubletype);
                    p = tree_cond_s(
                            tree_cmp_s(OP_GE, p, c, ty_ldoubletype),
                            tree_optree_s['+'](OP_ADD,
                                enode_cast_s(
                                    enode_cast_s(tree_sub_s(OP_SUB, p, c, ty_ldoubletype),
                                                 sty, f & ENODE_FECAST),
                                    stty, f & ENODE_FECAST),
                                tree_uconst_s((unsigned long)TG_INT_MAX + 1, stty),
                                NULL),
                            simp_cvtree_s(OP_CVF, fty, sty, p),
                            NULL);
                    p->f.cvfpu = 1;
                    p->f.npce = npce;
                    break;
                case 0:
                    assert(!"invalid conversion operator -- should never reach here");
                    break;
                default:
                    p = simp_cvtree_s(op, fty, stty, p);
                    break;
            }
            err_unmute();
        }
    }
    oty = TY_RMQENUM(tty->t.type);
    if (TY_ISSCHAR(oty) || oty == ty_shorttype)
        p = simp_cvtree_s(OP_CVI, stty, tty, p);
    else if (oty == ty_uchartype || oty == ty_chartype || oty == ty_ushorttype)
        p = simp_cvtree_s(OP_CVU, stty, tty, p);
    else if (TY_ISPTR(oty))
        p = simp_cvtree_s(OP_CVU, stty, tty, p);
    else if (oty == ty_floattype || oty == ty_doubletype)
        p = simp_cvtree_s(OP_CVF, stty, tty, p);
    else
        p = tree_retype_s(p, tty);

    ret:
        p->f.ecast |= (f & ENODE_FECAST);
        return p;
}


#define LEFT_HAS_RIGHTS_QUAL(l, r) ((l->op & r->op & TY_CONSVOL) == (r->op & TY_CONSVOL))

/*
 *  checks types for the = operator (type-tree version);
 *  void signals assignment of compatible incomplete types;
 *  ASSUMPTION: enum is always int
 */
ty_t *(enode_tcasgnty_s)(ty_t *ty, tree_t *r)
{
    int ret;
    ty_t *lty, *rty;

    assert(ty);
    assert(r);
    assert(r->type);
    assert(ty_voidtype);    /* ensures types initialized */

    lty = TY_UNQUAL(ty);
    rty = TY_UNQUAL(r->type);

    if ((TY_ISARITH(lty) && TY_ISARITH(rty)) ||
        ((TY_ISSTRUNI(lty) || lty->t.type == ty_voidtype) && lty->t.type == rty->t.type))
        return (lty->size == 0 || rty->size == 0)? ty_voidtype: lty;
    if (lty->size == 0 || rty->size == 0)
        return NULL;
    if (TY_ISPTR(lty) && enode_isnpc_s(r))
        return lty;
    if (((TY_ISVOIDP(lty) && TY_ISPTR(rty)) || (TY_ISPTR(lty) && TY_ISVOIDP(rty))) &&
        LEFT_HAS_RIGHTS_QUAL(lty->type, rty->type))
        return lty;
    if (TY_ISPTR(lty) && TY_ISPTR(rty) &&
        (ret = ty_equiv(TY_UNQUAL(lty->type), TY_UNQUAL(rty->type), 1)) != 0 &&
        LEFT_HAS_RIGHTS_QUAL(lty->type, rty->type)) {
        if (ret > 1)
            err_issue_s(ERR_EXPR_ASGNENUMPTR, ty, rty);
        return lty;
    }

    return NULL;
}


/*
 *  checks types for the = operator (tree-tree version);
 */
ty_t *(enode_tcasgn_s)(tree_t *l, tree_t *r)
{
    assert(l);

    return enode_tcasgnty_s(l->type, r);
}


/*
 *  checks types for the ?: operator;
 *  the type of the first operand should be checked elsewhere
 */
ty_t *(enode_tccond_s)(tree_t *l, tree_t *r)
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
    else if (TY_ISPTR(lty) && enode_isnpc_s(r))
        ty = lty;
    else if (enode_isnpc_s(l) && TY_ISPTR(rty))
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

    if ((TY_ISPTR(lty) && enode_isnpc_s(r)) ||
        (TY_ISPTR(lty) && !TY_ISFUNC(lty->type) && TY_ISVOIDP(rty)) ||
        (TY_ISPTR(lty) && TY_ISPTR(rty) && ty_equiv(lty, rty, 1)))
        return ty_voidptype;
    else if ((enode_isnpc_s(l) && TY_ISPTR(rty)) ||
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
ty_t *(enode_tcindir_s)(tree_t *p)
{
    ty_t *ty;

    assert(p);
    assert(p->type);

    ty = TY_UNQUAL(p->type);

    if (TY_ISPTR(ty) && (TY_ISFUNC(ty->type) || TY_ISARRAY(ty->type)))
        return ty->type;
    else {
        ty = ty_deref_s(ty);
        if (!ty)
            err_issue_s(ERR_EXPR_POINTER, p->type);
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
            return ty_qual_s(TY_CONST, ty_voidtype, 0);    /* always succeeds */
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

    p = enode_pointer_s(p);
    if (!TY_ISSCALAR(p->type)) {
        err_issuep(&p->pos, ERR_EXPR_CONDTYPE, (!op)? "conditional": name,
                   (!op)? "": optab[i].name, p->type);
        p = NULL;
    }

    return p;
}


/*
 *  prints a type error for an operator
 */
tree_t *(enode_tyerr_s)(int op, tree_t *l, tree_t *r)
{
    static struct {
        int op;
        const char *name;
    } optab[] = {
        OP_ASGN, "=",  OP_INDIR, "*",  OP_NEG,  "-",  OP_CALL, "()", OP_ADD,  "+",  OP_SUB,   "-",
        OP_LSH,  "<<", OP_MOD,   "%",  OP_RSH,  ">>", OP_BAND, "&",  OP_BCOM, "~",  OP_BOR,   "|",
        OP_BXOR, "^",  OP_DIV,   "/",  OP_MUL,  "*",  OP_EQ,   "==", OP_GE,   ">=", OP_GT,    ">",
        OP_LE,   "<=", OP_LT,    "<",  OP_NE,   "!=", OP_NOT,  "!",  OP_COND, "?:", OP_RIGHT, ",",
        OP_INCR, "++", OP_DECR,  "--", OP_SUBS, "[]",
        0,       NULL    /* checks for OP_AND and OP_OR done in enode_chkcond() */
    };

    int i;

    op = op_generic(op);
    for (i = 0; optab[i].op; i++)
        if (op == optab[i].op)
            break;
    assert(optab[i].name);

    if (r)
        err_issue_s(ERR_EXPR_BINOPERR, optab[i].name, l->type, r->type);
    else
        err_issue_s(ERR_EXPR_UNIOPERR, optab[i].name, l->type);

    return NULL;
}

/* end of enode.c */
