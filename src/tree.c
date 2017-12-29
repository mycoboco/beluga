/*
 *  tree handling
 */

#include <stddef.h>        /* NULL */
#include <stdio.h>         /* sprintf */
#include <string.h>        /* strlen */
#include <cbl/arena.h>     /* arena_t, ARENA_CALLOC, ARENA_ALLOC */
#include <cbl/assert.h>    /* assert */
#ifndef NDEBUG
#include <stdio.h>         /* FILE, fprintf, fputs, putc */
#include <string.h>        /* memcpy */
#endif    /* !NDEBUG */

#include "clx.h"
#include "common.h"
#include "enode.h"
#include "err.h"
#include "expr.h"
#include "ir.h"
#include "lex.h"
#include "lmap.h"
#include "main.h"
#include "op.h"
#include "simp.h"
#include "sset.h"
#include "strg.h"
#include "sym.h"
#include "ty.h"
#include "tree.h"

#define T(p) ((ty_t *)(p))    /* shorthand for cast to ty_t * */

/* resolves op codes for compound assignments */
#define COP(op) (((op) < OP_POS)? (op): cop[((op)-OP_CADD) >> OP_SOP])


/* tokens to tree-generating functions */
tree_t *(*tree_optree[])() = {
#define xx(a, b, c, d, e, f, g, h) e,
#define kk(a, b, c, d, e, f, g, h) e,
#define yy(a, b, c, d, e, f, g, h) e,
#include "xtoken.h"
};

/* tokens to tree opreators */
int tree_oper[] = {
#define xx(a, b, c, d, e, f, g, h) d,
#define kk(a, b, c, d, e, f, g, h) d,
#define yy(a, b, c, d, e, f, g, h) d,
#include "xtoken.h"
};


/* arena in which trees generated */
static arena_t **where = &strg_stmt;

/* table to resolve op codes for compound assignments */
static int cop[] = {
    OP_ADD, OP_SUB, OP_LSH, OP_MOD, OP_RSH, OP_BAND, OP_BOR, OP_BXOR, OP_DIV, OP_MUL
};

#ifndef NDEBUG
/* # of used nodes in parr */
static int pid;

/* information for printing tree */
static struct {
    const void *node;    /* for tree_t * and dag_node_t * */
    int printed;
} *parr;
#endif    /* !NDEBUG */


/*
 *  constructs new tree loci
 */
tree_pos_t *(tree_npos)(const lmap_t *l, const lmap_t *o, const lmap_t *r)
{
    tree_pos_t *p;

    assert(o);

    p = ARENA_ALLOC(*where, sizeof(*p));
    (*p)[0] = (l)? l: o;
    (*p)[1] = o;
    (*p)[2] = (r)? r: o;

    return p;
}


/*
 *  constructs new tree loci with a single locus
 */
tree_pos_t *(tree_npos1)(const lmap_t *p)
{
    return tree_npos(p, p, p);
}


/*
 *  constructs a new tree
 */
tree_t *(tree_new)(int op, ty_t *ty, tree_t *l, tree_t *r, tree_pos_t *pos)
{
    tree_t *p;

    assert(ty);
    assert(pos);
    assert(where);

    p = ARENA_CALLOC(*where, sizeof(*p), 1);
    p->op = op;
    p->type = ty;
    p->kid[0] = l;
    p->kid[1] = r;
    p->pos = pos;
    p->orgn = p;

    return p;
}


/*
 *  constructs a tree for an expression in a given arena
 */
tree_t *(tree_texpr)(tree_t *(*f)(int, int, const lmap_t *), int tok, arena_t *a,
                     const lmap_t *posm)
{
    arena_t **save = where;
    tree_t *p;

    assert(f);
    assert(a);

    where = &a;
    p = f(tok, 0, posm);
    where = save;

    return p;
}


/*
 *  finds the rightmost non-RIGHT tree
 */
tree_t *(tree_rightkid)(tree_t *p)
{
    while (p && p->op == OP_RIGHT) {
        assert(p->kid[1]);
        p = p->kid[1];
    }
    assert(p);

    return p;
}


/*
 *  extracts a sub-tree with a side effect;
 *  ASSUMPTION: access to volatile object may be elided
 */
tree_t *(tree_root)(tree_t *p)
{
    tree_t *l, *r;

    assert(ty_voidtype);    /* ensures types initialized */

    if (!p)
        return p;

    switch(op_generic(p->op)) {
        case OP_CNST:
        case OP_ADDRG:
        case OP_ADDRF:
        case OP_ADDRL:
            if (main_opt()->std != 1 || !simp_needconst)
                p = NULL;
            break;
        case OP_ARG:
        case OP_ASGN:
        case OP_CALL:
        case OP_RET:
        case OP_JMP:
            break;
        case OP_INDIR:
        case OP_CVF:
        case OP_CVI:
        case OP_CVU:
        case OP_CVP:
        case OP_NEG:
        case OP_BCOM:
        case OP_NOT:
        case OP_FIELD:
            p = tree_root(p->kid[0]);
            break;
        case OP_ADD:
        case OP_SUB:
        case OP_LSH:
        case OP_MOD:
        case OP_RSH:
        case OP_BAND:
        case OP_BOR:
        case OP_BXOR:
        case OP_DIV:
        case OP_MUL:
        case OP_EQ:
        case OP_GE:
        case OP_GT:
        case OP_LE:
        case OP_LT:
        case OP_NE:
            l = tree_root(p->kid[0]);
            r = tree_root(p->kid[1]);
            p = (l && r)? tree_new(OP_RIGHT, p->type, l, r, p->orgn->pos):
                (l)? l: r;    /* tree_right() not used here */
            break;
        case OP_AND:
        case OP_OR:
            if ((p->kid[1] = tree_root(p->kid[1])) == NULL)
                p = tree_root(p->kid[0]);
            break;
        case OP_COND:
            {
                assert(p->kid[1]);
                assert(p->kid[1]->op == OP_RIGHT);

                if (p->f.cvfpu) {
                    p = tree_root(p->kid[0]->kid[0]);
                    break;
                }
                r = p->kid[1];
                r->kid[0] = (p->u.sym && r->kid[0] && op_generic(r->kid[0]->op) == OP_ASGN)?
                                tree_root(r->kid[0]->kid[1]): tree_root(r->kid[0]);
                r->kid[1] = (p->u.sym && r->kid[1] && op_generic(r->kid[1]->op) == OP_ASGN)?
                                tree_root(r->kid[1]->kid[1]): tree_root(r->kid[1]);
                if (!r->kid[0] && !r->kid[1])
                    p = tree_root(p->kid[0]);
                else
                    p->u.sym = NULL;    /* sym no longer used */
            }
            break;
        case OP_RIGHT:
            if (p->kid[0] && p->kid[0]->op == OP_RIGHT &&
                tree_untype(p->kid[1]) == tree_untype(p->kid[0]->kid[0])) {
                p = p->kid[0]->kid[1];
                break;
            }
            if (p->type->t.type == ty_voidtype && !p->kid[0] && p->kid[1]) {
                if ((p->kid[1] = tree_root(p->kid[1])) == NULL)
                    p = NULL;
                break;
            }
            p->kid[0] = tree_root(p->kid[0]);
            p->kid[1] = tree_root(p->kid[1]);
            p = (!p->kid[0])? p->kid[1]: (!p->kid[1])? p->kid[0]: p;
            break;
        default:
            assert("invalid operation code -- should never reach here");
            break;
    }

    return p;
}


/*
 *  constructs a tree with a type/pos changed
 */
tree_t *(tree_retype)(tree_t *p, ty_t *ty, tree_pos_t *tpos)
{
    tree_t *q, *r;

    assert(p);

    if (!ty)    /* always copies */
        ty = p->type;
    else if (p->type == ty && (!tpos || p->orgn->pos == tpos))    /* copies when necessary */
        return p;
    if (!tpos)
        tpos = p->orgn->pos;

    q = tree_new(p->op, ty, p->kid[0], p->kid[1], tpos);
    q->f = p->f;
    q->u = p->u;
    if (p != p->orgn) {
        r = p->orgn;
        q->orgn = tree_new(r->op, q->type, r->kid[0], r->kid[1], tpos);
        q->orgn->f = p->orgn->f;
    }
    q->kid[2] = p;    /* used by tree_untype() */

    return q;
}


/*
 *  inspects if a tree calls a function that returns a structure/union
 */
int (tree_iscallb)(const tree_t *e)
{
    assert(e);
    assert(!(e->op == OP_RIGHT && e->kid[0] && e->kid[1] && e->kid[0]->op == OP_CALL+OP_B) ||
           op_generic(e->kid[1]->op) == OP_INDIR);

    return (e->op == OP_RIGHT && e->kid[0] && e->kid[1] &&
            e->kid[0]->op == OP_CALL+OP_B && e->kid[1]->op == OP_INDIR+OP_B &&
            OP_ISADDR(e->kid[1]->kid[0]->op) && e->kid[1]->kid[0]->u.sym);
}


/*
 *  extracts the name of a function from a tree if any
 */
const char *(tree_fname)(tree_t *f)
{
    char *name;

    assert(f);

    f = tree_rightkid(f);
    assert(f->type->type);
    if (OP_ISADDR(f->op) && !GENSYM(f->u.sym)) {
        name = ARENA_ALLOC(strg_func, strlen(f->u.sym->name)+1+2);
        sprintf(name, "`\1%s\2'", f->u.sym->name);
        return name;
    }

    return "a function";
}


/*
 *  constructs a tree to access a struct/union object from another tree;
 *  ASSUMPTION: struct/union values are carried in temporary objects
 */
static tree_t *addrof(tree_t *p)
{
    tree_t *q = p;

    assert(p);

    while (1)
        switch(op_generic(q->op)) {
            case OP_RIGHT:
            case OP_ASGN:
                assert(q->kid[1]);
                q = q->kid[1];
                continue;
            case OP_COND:
                {
                    sym_t *t1 = q->u.sym;
                    assert(t1);
                    q->u.sym = NULL;
                    q = tree_id(t1, p->orgn->pos);
                }
                /* no break */
            case OP_INDIR:
                if (p == q)
                    return tree_retype(p->kid[0], NULL, p->orgn->pos);
                q = q->kid[0];
                q = tree_right(p, q, q->type, p->orgn->pos);
                q->f.nlval = 1;
                goto ret;
            default:
                assert(!"invalid operation code -- should never reach here");
                goto ret;
        }

    ret:
        assert(q);
        return q;
}


/*
 *  constructs a RIGHT tree;
 *  root() applies to the left;
 *  pointer() applies to the right;
 *  left child is NULL if only one child exists
 */
tree_t *(tree_right)(tree_t *l, tree_t *r, ty_t *ty, tree_pos_t *tpos)
{
    if (!l && !r)
        return NULL;

    if (!r) {
        r = l;
        l = NULL;
    }
    if (l)
        l = enode_pointer(l);
    r = enode_pointer(r);

    if (!ty)
        ty = r->type;
    ty = TY_UNQUAL(ty);

    /* cannot remove type check to preserve rvalue-ness */
    if (!l && r->op == OP_RIGHT && ty_same(ty, r->type))
        return tree_retype(r, ty, tpos);
    return tree_new(OP_RIGHT, ty, l, r, tpos);
}


/*
 *  constructs an ASGN tree;
 *  value() applies to the right;
 *  pointer() applies to both;
 *  ASSUMPTION: bit-field is singed or unsigned int;
 *  ASSUMPTION: overflow from left shift is benign on the target
 */
static tree_t *asgn(int op, tree_t *l, tree_t *r, ty_t *ty, int force, tree_pos_t *tpos)
{
    ty_t *aty;
    tree_t *ll;

    assert(op == OP_ASGN  || op == OP_INCR || op == OP_DECR || op == OP_CADD  || op == OP_CSUB ||
           op == OP_CLSH  || op == OP_CMOD || op == OP_CRSH || op == OP_CBAND || op == OP_CBOR ||
           op == OP_CBXOR || op == OP_CDIV || op == OP_CMUL);
    assert(tpos);
    assert(ty_voidtype);    /* ensures types initialized */

    if (!l || !r)
        return NULL;

    l = enode_pointer(l);
    r = enode_value(enode_pointer(r));

    if (l->op != OP_FIELD) {
        ll = tree_addr(l, NULL, 0, tpos);
        if (!ll)
            return NULL;
    } else
        ll = l;

    if (!ty)
        ty = enode_tcasgn(l, r, tpos);
    if (ty == ty_voidtype) {
        err_dtpos(tpos, l, r, ERR_EXPR_ASGNINCOMP);
        return NULL;
    } else if (ty) {
        if (TY_ISSCALAR(ty))
            r = enode_cast(r, ty, 1, TREE_PO(tpos));
    } else if (TY_ISARRAY(l->type) || op == OP_INCR || op == OP_DECR)
        ty = l->type;
    else
        return enode_tyerr(OP_ASGN, l, r, tpos);
    ty = TY_UNQUAL(ty);

    l = ll;
    aty = l->type;
    if (TY_ISPTR(aty))
        aty = TY_UNQUAL(aty)->type;
    if (!force && (TY_ISCONST(aty) || TY_HASCONST(aty)))
        err_dtpos(tpos, l, NULL, ERR_EXPR_ASGNCONST,
                  (OP_ISADDR(l->op))? l->u.sym: NULL, " location");
    if (l->op == OP_FIELD) {
        tree_pos_t *rpos;
        int n = TG_CHAR_BIT*l->u.field->type->size - SYM_FLDSIZE(l->u.field);
        if (n > 0) {
            rpos = r->orgn->pos;
            if (TY_UNQUAL(l->u.field->type) == ty_unsignedtype)
                r = tree_bit(OP_BAND,
                             r, tree_uconst(SYM_FLDMASK(l->u.field), ty_unsignedtype, rpos),
                             ty_unsignedtype, rpos);
            else {
                assert(TY_UNQUAL(l->u.field->type) == ty_inttype);
                if (op_generic(r->op) == OP_CNST) {
                    if (!sym_infld(r->u.v.s, l->u.field))
                        err_dtpos(tpos, l, r, ERR_EXPR_BIGFLD);
                    r = tree_sconst(sym_sextend(r->u.v.s, l->u.field), ty_inttype, rpos);
                } else
                    r = tree_sha(OP_RSH,
                            tree_sha(OP_LSH,
                                r, tree_sconst(xis(n), ty_inttype, rpos), ty_inttype, rpos),
                            tree_sconst(xis(n), ty_inttype, rpos),
                            ty_inttype, rpos);
            }
        }
    }

    if (TY_ISSTRUNI(ty) && OP_ISADDR(l->op) && tree_iscallb(r))
        return tree_right(    /* tree_call() cannot be used here */
                   tree_new(OP_CALL+OP_B, ty, r->kid[0]->kid[0], l, tpos),
                   tree_id(l->u.sym, tpos), ty, tpos);
    return tree_new(OP_ASGN+op_sfxs(ty), ty, l, r, tpos);
}


/*
 *  constructs an ASGN tree
 */
tree_t *(tree_asgn)(int op, tree_t *l, tree_t *r, ty_t *ty, tree_pos_t *tpos)
{
    return asgn(op, l, r, ty, 0, tpos);
}


/*
 *  constructs an ASGN tree ignoring const qualification
 */
tree_t *(tree_asgnf)(int op, tree_t *l, tree_t *r, ty_t *ty, tree_pos_t *tpos)
{
    return asgn(op, l, r, ty, 1, tpos);
}


/*
 *  constructs a tree to assign an expression to a symbol;
 *  const on the symbol ignored in constructing a tree
 */
tree_t *(tree_asgnid)(sym_t *p, tree_t *e, tree_pos_t *tpos)
{
    tree_pos_t *ppos;

    assert(p);
    assert(p->type);

    if (!e)
        return NULL;

    ppos = (p->pos)? tree_npos1(p->pos): tpos;
    return (TY_ISARRAY(p->type))?
               tree_new(OP_ASGN+OP_B, p->type,
                        tree_id(p, ppos),
                        tree_new(OP_INDIR+OP_B, e->type, e, NULL, e->orgn->pos), tpos):
               asgn(OP_ASGN, tree_id(p, ppos), e, NULL, 1, tpos);
}


/*
 *  constructs a tree for compound assignments;
 *  a preset result type ty is not used thus not delivered;
 *  note that the first parameter is not op code but token code
 */
tree_t *(tree_casgn)(int tc, tree_t *v, tree_t *e, tree_pos_t *tpos)
{
    v = asgn(tree_oper[tc],
             v,
             tree_optree[tc](tree_oper[tc], v, e, NULL, tpos),
             NULL, 0, tpos);
    if (v)
        v->orgn->f.paren = 1;

    return v;
}


#define SETNPCE(p)                                                  \
    do {                                                            \
        npce |= (p)->f.npce;                                        \
        if (op_generic((p)->op) == OP_CNST)                         \
            switch(op_type((p)->op)) {                              \
                case OP_F: npce |= TREE_FICE; break;                \
                case OP_I:                                          \
                case OP_U: break;                                   \
                default:   npce |= (TREE_FACE|TREE_FICE); break;    \
            }                                                       \
        else npce |= (TREE_FACE|TREE_FICE);                         \
    } while(0)

/*
 *  constructs a COND tree;
 *  cond() applies to the first;
 *  pointer() applies to the first in enode_chkcond();
 *  pointer(), value() and root() apply to the second and third
 */
tree_t *(tree_cond)(tree_t *e, tree_t *l, tree_t *r, ty_t *ty, tree_pos_t *tpos)
{
    tree_t *p;
    sym_t *t1;

    assert(ty_ldoubletype);    /* ensures types initialized */

    if (!e || !l || !r)
        return NULL;

    if ((e = enode_chkcond(OP_COND, e, "first operand of ")) == NULL)
        return NULL;
    l = enode_value(enode_pointer(l));
    r = enode_value(enode_pointer(r));

    if (!ty) {
        ty = enode_tccond(l, r);
        if (!ty)
            return enode_tyerr(OP_COND, l, r, tpos);
    }
    assert(!TY_ISQUAL(ty));

    if (op_generic(e->op) == OP_CNST) {
        tree_t *o = e;
        int npce = e->f.npce;
        switch(op_type(e->op)) {
            case OP_P:
                e = enode_cast((xt(e->u.v.p))? l: r, ty, 0, NULL);
                npce |= (TREE_FADDR|TREE_FACE|TREE_FICE);
                break;
            case OP_F:
                e = (enode_cast(e, ty_ldoubletype, 0, NULL)->u.v.ld)? l: (p=r, r=l, l=p);
                npce |= TREE_FICE;
                goto branch;
            default:
#ifdef SUPPORT_LL
                e = (xt(enode_cast(e, ty_ullongtype, 0, NULL)->u.v.u))? l: (p=r, r=l, l=p);
#else    /* !SUPPORT_LL */
                e = (xt(enode_cast(e, ty_ulongtype, 0, NULL)->u.v.u))? l: (p=r, r=l, l=p);
#endif    /* SUPPORT_LL */
                /* no break */
            branch:
                e = enode_cast(e, ty, 0, NULL);
                r->f.npce &= (TREE_FACE|TREE_FICE);    /* masks COMMA|ADDR */
                if (main_opt()->std == 1)
                    npce |= tree_chkinit(r);    /* ADDR */
                SETNPCE(l);
                SETNPCE(r);
                break;
        }
        if (op_generic(e->op) == OP_CNST || op_generic(e->op) == OP_ADDRG)
            e->f.npce = npce;
        e = TREE_RVAL(e, tpos);
        e->orgn = tree_new(OP_COND, ty, o, tree_new(OP_RIGHT, ty, l, r, tpos), tpos);
        return e;
    }
    if (ty->t.type != ty_voidtype && ty->size > 0) {
        t1 = sym_new(SYM_KTEMP, LEX_REGISTER, ty, sym_scope);
        l = tree_asgnid(t1, l, tpos);
        r = tree_asgnid(t1, r, tpos);
    } else
        t1 = NULL;
    p = tree_new(OP_COND, ty, enode_cond(e),    /* tree_right() cannot be used here */
                              tree_new(OP_RIGHT, ty, l, r, tpos), tpos);
                              /* RIGHT tree may have no children */
    p->u.sym = t1;

    return p;
}

#undef SETNPCE


/*
 *  constructs an AND or OR tree;
 *  pointer() applies to both in enode_chkcond();
 *  cond() applies to both
 */
tree_t *(tree_and)(int op, tree_t *l, tree_t *r, ty_t *ty, tree_pos_t *tpos)
{
    assert(op == OP_AND || op == OP_OR);

    if (!l || !r ||
        (l = enode_chkcond(op, l, "left operand of ")) == NULL ||
        (r = enode_chkcond(op, r, "right operand of ")) == NULL)
        return NULL;

    if (!ty) {
        ty = enode_tcand(l, r);
        if (!ty)
            return enode_tyerr(op, l, r, tpos);
    }
    assert(!TY_ISQUAL(ty));
    /* no separate expr function for OR; thus checked here */
    if (op == OP_OR) {
        if (l->orgn->op == OP_AND && !l->orgn->f.paren)
            err_dmpos(TREE_TW(l), ERR_EXPR_NEEDPAREN, TREE_PO(tpos), NULL);
        if (r->orgn->op == OP_AND && !r->orgn->f.paren)
            err_dmpos(TREE_TW(r), ERR_EXPR_NEEDPAREN, TREE_PO(tpos), NULL);
    }

    return simp_tree(op, ty, enode_cond(l), enode_cond(r), tpos);
}


/*
 *  constructs a BAND, BOR, BXOR or MOD tree;
 *  pointer() and value() apply to both
 */
tree_t *(tree_bit)(int op, tree_t *l, tree_t *r, ty_t *ty, tree_pos_t *tpos)
{
    ty_t *uty;

    assert(op == OP_BAND  || op == OP_BOR  || op == OP_BXOR  || op == OP_MOD ||
           op == OP_CBAND || op == OP_CBOR || op == OP_CBXOR || op == OP_CMOD);

    if (!l || !r)
        return NULL;

    l = enode_value(enode_pointer(l));
    r = enode_value(enode_pointer(r));

    if (!ty) {
        ty = enode_tcbit(l, r);
        if (!ty)
            return enode_tyerr(op, l, r, tpos);
    }
    assert(!TY_ISQUAL(ty));

    l = enode_cast(l, ty, 0, NULL);
    r = enode_cast(r, ty, 0, NULL);
    op = COP(op);
    if (op != OP_MOD) {
        if (OP_ISCMP(l->orgn->op) && !l->orgn->f.paren)
            err_dmpos(TREE_TW(l), ERR_EXPR_NEEDPAREN, TREE_PO(tpos), NULL);
        if (OP_ISCMP(r->orgn->op) && !r->orgn->f.paren)
            err_dmpos(TREE_TW(r), ERR_EXPR_NEEDPAREN, TREE_PO(tpos), NULL);
        uty = ty_ucounter(l->type);
        l = enode_cast(l, uty, 0, NULL);
        r = enode_cast(r, uty, 0, NULL);
    }

    if (op == OP_MOD)
        return simp_tree(op, ty, l, r, tpos);
    return enode_cast(simp_tree(op, uty, l, r, tpos), ty, 0, NULL);
}


/*
 *  constructs a LE, GE, LT, GT, EQ or NE tree;
 *  pointer() and value() apply to both;
 *  ASSUMPTION: pointers are converted to integer for comparison
 */
tree_t *(tree_cmp)(int op, tree_t *l, tree_t *r, ty_t *ty, tree_pos_t *tpos)
{
    int npce = 0;

    assert(op == OP_LE || op == OP_GE || op == OP_LT || op == OP_GT || op == OP_EQ || op == OP_NE);
    assert(ty_voidtype);    /* ensures types initialized */

    if (!l || !r)
        return NULL;

    l = enode_value(enode_pointer(l));
    r = enode_value(enode_pointer(r));

    if (!ty) {
        ty = (op == OP_EQ || op == OP_NE)? enode_tceq(l, r): enode_tccmp(l, r);
        if (!ty)
            return enode_tyerr(op, l, r, tpos);
    }

    ty = TY_RMQENUM(ty);    /* ty->op used below */
    if (ty == ty_voidtype)
        return tree_cmp(op, r, l, NULL, tpos);
    if (ty == ty_voidptype) {
        ty = ty_ptruinttype;
        npce = (TREE_FADDR|TREE_FACE|TREE_FICE);    /* catches comparing NPCs */
    } else if (TY_ISFP(ty))
        npce = TREE_FICE;
    l = enode_cast(l, ty, 0, NULL);
    r = enode_cast(r, ty, 0, NULL);

    l = simp_tree(op + ty->op, ty_inttype, l, r, tpos);
    if (op_generic(l->op) == OP_CNST && npce)
        l->f.npce |= npce;

    return l;
}


/*
 *  constructs a RSH or LSH tree;
 *  pointer() and value() apply to both;
 *  ASSUMPTION: RSH always performs arithmetic shift on the target
 */
tree_t *(tree_sha)(int op, tree_t *l, tree_t *r, ty_t *ty, tree_pos_t *tpos)
{
    assert(op == OP_RSH  || op == OP_LSH ||
           op == OP_CRSH || op == OP_CLSH);
    assert(ty_inttype);    /* ensures types initialized */

    if (!l || !r)
        return NULL;

    l = enode_value(enode_pointer(l));
    r = enode_value(enode_pointer(r));

    if (!ty) {
        ty = enode_tcsh(l, r);
        if (!ty)
            return enode_tyerr(op, l, r, tpos);
    }
    assert(!TY_ISQUAL(ty));

    l = enode_cast(l, ty, 0, NULL);
    r = enode_cast(r, ty_inttype, 0, NULL);

    return simp_tree(COP(op), ty, l, r, tpos);
}


/*
 *  constructs a RSH or LSH tree;
 *  unsigned shift used when logical shift turned on for RSH
 */
tree_t *(tree_sh)(int op, tree_t *l, tree_t *r, ty_t *ty, tree_pos_t *tpos)
{
    ty_t *lty;

    assert(tpos);

    if (!l || !r)
        return NULL;

    lty = ty_ipromote(l->type);

    if (!TY_ISINTEGER(lty))
        return tree_sha(op, l, r, ty, tpos);
    if (main_opt()->logicshift && op == OP_RSH && !TY_ISUNSIGN(lty)) {
        /* logical shift has no chance to warn so copied from warnnegrsh() in simp.c */
        if (op_generic(l->op) == OP_CNST && xls(l->u.v.s, xO))
            err_dtpos(tpos, l, NULL, ERR_EXPR_RSHIFTNEG);
        l = enode_cast(l, ty_ucounter(lty), 0, NULL);
    }
    l = tree_sha(op, l, r, ty, tpos);
    return (l)? enode_cast(l, lty, 0, TREE_PO(tpos)): NULL;
}


/*
 *  constructs an ADD tree;
 *  pointer() and value() apply to both;
 *  ASSUMPTION: pointer arithmetic can be implemented using integers;
 *  ASSUMPTION: all pointers are uniform (same representation)
 */
tree_t *(tree_add)(int op, tree_t *l, tree_t *r, ty_t *ty, tree_pos_t *tpos)
{
    ty_t *gty = ty;

    assert(op == OP_ADD || op == OP_INCR || op == OP_SUBS ||
           op == OP_CADD);
    assert(ty_voidtype);    /* ensures types initialized */

    if (!l || !r)
        return NULL;

    l = enode_value(enode_pointer(l));
    r = enode_value(enode_pointer(r));

    if (op == OP_INCR || op == OP_SUBS) {
        if (TY_ISARRAY(l->type)) {
            assert(l->op == OP_RIGHT);
            return l;
        } else if (TY_ISARRAY(r->type)) {
            assert(r->op == OP_RIGHT);
            return r;
        }
    }
    if (!ty) {
        ty = enode_tcadd(l, r);
        if (!ty)
            return (op == OP_INCR)?
                enode_tyerr(op, (op_optype(l->op) == OP_CNST+OP_I && xe(l->u.v.s, xI))? r: l,
                            NULL, tpos):
                enode_tyerr(op, l, r, tpos);
    }

    if (ty == ty_voidtype)
        return tree_add(op, r, l, NULL, tpos);
    else if (TY_ISARITH(ty)) {
        l = enode_cast(l, ty, 0, NULL);
        r = enode_cast(r, ty, 0, NULL);
    } else {
        if (!gty) {    /* do math only when no given type */
            ssz_t n;    /* signed because of negative indices */
            n = ty->type->size;
            if (n == 0)
                err_dmpos(TREE_TW(r), ERR_EXPR_UNKNOWNSIZE, TREE_PO(tpos), NULL, ty->type);
            else if (l->f.npce & TREE_FICE)
                op = 0;
            l = enode_cast(l, ty_ipromote(l->type), 0, NULL);
            if (n > 1)
                l = tree_mul(OP_MUL, tree_sconst(xis(n), ty_ptrsinttype, tpos), l, NULL, tpos);
        }
        l = simp_tree(OP_ADD + TY_POINTER, ty, l, r, tpos);
        if (op_generic(l->op) == OP_ADDRG && !op)
            l->f.npce |= TREE_FADDR;
        return l;
    }

    l = simp_tree(OP_ADD, ty, l, r, tpos);
    if (op_generic(l->op) == OP_CNST && TY_ISFP(ty))
        l->f.npce |= TREE_FICE;

    return l;
}


/*
 *  construct a SUB tree;
 *  pointer() and value() apply to both;
 *  ASSUMPTION: pointer arithmetic can be implemented using integers;
 *  ASSUMPTION: signed/unsigned integers are compatible on the target;
 *  ASSUMPTION: all pointers are uniform (same representation)
 */
tree_t *(tree_sub)(int op, tree_t *l, tree_t *r, ty_t *ty, tree_pos_t *tpos)
{
    ssz_t n;

    assert(op == OP_SUB || op == OP_DECR ||
           op == OP_CSUB);
    assert(!ty || TY_ISARITH(ty));    /* given type should be arithmetic if any */
    assert(ty_ptrsinttype);           /* ensures types initialized */

    if (!l || !r)
        return NULL;

    l = enode_value(enode_pointer(l));
    r = enode_value(enode_pointer(r));

    if (op == OP_DECR || op == OP_SUBS) {
        if (TY_ISARRAY(l->type)) {
            assert(l->op == OP_RIGHT);
            return l;
        } else if (TY_ISARRAY(r->type)) {
            assert(r->op == OP_RIGHT);
            return r;
        }
    }
    if (!ty) {
        ty = enode_tcsub(l, r);
        if (!ty)
            return (op == OP_DECR)?
                enode_tyerr(op, (op_optype(l->op) == OP_CNST+OP_I && xe(l->u.v.s, xI))? r: l,
                            NULL, tpos):
                enode_tyerr(op, l, r, tpos);
    }

    if (TY_ISPTR(ty)) {
        ty = TY_UNQUAL(ty);
        n = ty->type->size;
        if (n == 0)
            err_dmpos(TREE_TW(l), ERR_EXPR_UNKNOWNSIZE, TREE_PO(tpos), NULL, ty->type);
        else if (r->f.npce & TREE_FICE)
            op = 0;
        r = enode_cast(r, ty_ipromote(r->type), 0, NULL);
        if (n > 1)
            r = tree_mul(OP_MUL, tree_sconst(xis(n), ty_ptrsinttype, tpos), r, NULL, tpos);
        r = simp_tree(OP_SUB + TY_POINTER, ty, l, r, tpos);
        if (op_generic(r->op) == OP_ADDRG && !op)
            r->f.npce |= TREE_FADDR;
        return r;
    } else if (ty == ty_voidtype) {
        ty = TY_UNQUAL(l->type);
        n = ty->type->size;
        if (n == 0) {
            err_dmpos(TREE_TW(l), ERR_EXPR_UNKNOWNSIZE, TREE_PO(tpos), NULL, ty->type);
            n = 1;    /* to avoid divide by 0 */
        } else
            ty = NULL;
        l = simp_tree(OP_SUB, ty_ptruinttype, enode_cast(l, ty_ptruinttype, 0, NULL),
                                              enode_cast(r, ty_ptruinttype, 0, NULL), tpos);
        l = enode_cast(
                tree_mul(OP_DIV, enode_cast(l, ty_ptrsinttype, 0, NULL),
                                 tree_sconst(xis(n), ty_ptrsinttype, tpos),
                         ty_ptrsinttype, tpos),
                ty_ptrdifftype, 0, NULL);
        if (op_generic(l->op) == OP_CNST && !ty)
            l->f.npce |= TREE_FADDR;
        return l;
    } else {
        l = enode_cast(l, ty, 0, NULL);
        r = enode_cast(r, ty, 0, NULL);
    }

    l = simp_tree(OP_SUB, ty, l, r, tpos);
    if (op_generic(l->op) == OP_CNST && TY_ISFP(ty))
        l->f.npce |= TREE_FICE;

    return l;
}


/*
 *  construct a MUL or DIV tree;
 *  pointer() and value() apply to both
 */
tree_t *(tree_mul)(int op, tree_t *l, tree_t *r, ty_t *ty, tree_pos_t *tpos)
{
    assert(op == OP_MUL  || op == OP_DIV ||
           op == OP_CMUL || op == OP_CDIV);

    if (!l || !r)
        return NULL;

    l = enode_value(enode_pointer(l));
    r = enode_value(enode_pointer(r));

    if (!ty) {
        ty = enode_tcmul(l, r);
        if (!ty)
            return enode_tyerr(op, l, r, tpos);
    }
    assert(!TY_ISQUAL(ty));

    l = enode_cast(l, ty, 0, NULL);
    r = enode_cast(r, ty, 0, NULL);

    l = simp_tree(COP(op), ty, l, r, tpos);
    if (op_generic(l->op) == OP_CNST && TY_ISFP(ty))
        l->f.npce |= TREE_FICE;

    return l;
}


/*
 *  constructs a tree for the unary + operation;
 *  pointer() and value() apply to the operand;
 */
tree_t *(tree_pos)(tree_t *p, ty_t *ty, tree_pos_t *tpos)
{
    if (!p)
        return NULL;

    p = enode_value(enode_pointer(p));

    if (!ty) {
        ty = enode_tcposneg(p);
        if (!ty)
            return enode_tyerr(OP_ADD, p, NULL, tpos);
    }

    p = enode_cast(p, ty, 0, NULL);

    p = simp_tree(OP_POS, ty, p, NULL, tpos);
    if (op_generic(p->op) == OP_CNST && TY_ISFP(ty))
        p->f.npce |= TREE_FICE;
    else if (TREE_ISLVAL(p))
        p = tree_right(NULL, p, NULL, tpos);    /* rvalue */

    return p;
}


/*
 *  constructs a NEG tree;
 *  pointer() and value() apply to the operand;
 *  ASSUMPTION: (unsigned)(-(signed)value) implements negation of unsigned
 */
tree_t *(tree_neg)(tree_t *p, ty_t *ty, tree_pos_t *tpos)
{
    assert(tpos);

    if (!p)
        return NULL;

    p = enode_value(enode_pointer(p));

    if (!ty) {
        ty = enode_tcposneg(p);
        if (!ty)
            return enode_tyerr(OP_SUB, p, NULL, tpos);
    }

    p = enode_cast(p, ty, 0, NULL);
    if (TY_ISUNSIGN(p->type)) {
        ty_t *sty = ty_scounter(p->type);
        err_dtpos(tpos, p, NULL, ERR_EXPR_NEGUNSIGNED, NULL);
        err_mute();    /* ERR_EXPR_NEGUNSIGNED is enough */
        p = simp_tree(OP_NEG, sty, enode_cast(p, sty, 0, NULL), NULL, tpos);
        p = enode_cast(p, ty, 0, NULL);
        err_unmute();
    } else {
        p = simp_tree(OP_NEG, p->type, p, NULL, tpos);
        if (op_generic(p->op) == OP_CNST && TY_ISFP(ty))
            p->f.npce |= TREE_FICE;
    }

    return p;
}


/*
 *  constructs a BCOM tree;
 *  pointer() and value() apply to the operand
 */
tree_t *(tree_bcom)(tree_t *p, ty_t *ty, tree_pos_t *tpos)
{
    if (!p)
        return NULL;

    p = enode_value(enode_pointer(p));

    if (!ty) {
        ty = enode_tcbcom(p);
        if (!ty)
            return enode_tyerr(OP_BCOM, p, NULL, tpos);
    }

    p = simp_tree(OP_BCOM, ty, enode_cast(p, ty, 0, NULL), NULL, tpos);

    return p;
}


/*
 *  constructs a NOT tree;
 *  pointer() and cond() applies to the operand
 */
tree_t *(tree_not)(tree_t *p, ty_t *ty, tree_pos_t *tpos)
{
    if (!p)
        return NULL;

    p = enode_pointer(p);

    if (!ty && (ty = enode_tcnot(p)) == NULL)
        return enode_tyerr(OP_NOT, p, NULL, tpos);

    p = simp_tree(OP_NOT, ty, enode_cond(p), NULL, tpos);

    return p;
}


/*
 *  constructs an INDIR tree;
 *  pointer() and value() apply to the operand;
 *  TODO: warn of breaking anti-aliasing rules;
 *  TODO: warn of referencing uninitialized objects
 */
tree_t *(tree_indir)(tree_t *p, ty_t *ty, int explicit, tree_pos_t *tpos)
{
    if (!p)
        return NULL;

    p = enode_value(enode_pointer(p));

    if (!ty && (ty = enode_tcindir(p, tpos)) == NULL)
        return NULL;

    if (TY_ISFUNC(ty) || TY_ISARRAY(ty))
        p = tree_retype(p, ty, tpos);
    else {
        p = tree_new(OP_INDIR+op_sfxs(ty), ty, p, NULL, tpos);
        p->f.eindir = explicit;
    }

    return p;
}


/*
 *  constructs an address(lvalue) tree
 */
tree_t *(tree_addr)(tree_t *p, ty_t *ty, int explicit, tree_pos_t *tpos)
{
    assert(tpos);
    assert(ty_voidtype);    /* ensures types initialized */

    if (!p)
        return NULL;

    if (!ty) {
        ty = enode_tcaddr(p);
        if (!ty) {
            err_dtpos(tpos, p, NULL,
                      (op_generic(p->op) == OP_FIELD)? ERR_EXPR_ADDRFLD: ERR_EXPR_NEEDLVALUE);
            return NULL;
        } else if (explicit && TY_ISVOID(ty))
            err_dtpos(tpos, p, NULL,
                      (ty == ty_voidtype)? ERR_EXPR_VOIDLVALUES: ERR_EXPR_VOIDLVALUENS);
    }

    if (TY_ISPTR(ty) && (TY_ISFUNC(ty->type) || TY_ISARRAY(ty->type)))
        return tree_retype(p, ty, (explicit)? tpos: NULL);

    return (explicit)? tree_retype(p->kid[0], NULL, tpos):
                       tree_retype(p->kid[0], NULL, p->orgn->pos);
}


/*
 *  constructs a CALL tree after arguments parsed;
 *  ASSUMPTION: pointers can be returned as an integer;
 *  ASSUMPTION: see TY_WIDEN() for additional assumptions
 */
static tree_t *call(tree_t *f, ty_t *ty, tree_t *args, sym_t *t3, tree_pos_t *tpos)
{
    tree_t *p;

    assert(f);
    assert(ty);
    assert(ty_ptruinttype);    /* ensures types initialized */

    if (args)
        f = tree_right(args, f, f->type, tpos);
    if (TY_ISSTRUNI(ty)) {
        assert(t3);
        p = tree_right(tree_new(OP_CALL+OP_B, ty,
                                f, tree_addr(tree_id(t3, tpos),
                                             ty_ptr(t3->type), 0, tpos), tpos),
                       tree_id(t3, tpos), ty, tpos);
    } else {
        ty_t *rty = TY_UNQUAL(ty);
        if (TY_ISPTR(ty))
            rty = ty_ptruinttype;
        p = tree_new(OP_CALL+OP_SFXW(rty), ty_ipromote(ty), f, NULL, tpos);
        if (TY_ISPTR(ty) || p->type->size > ty->size)
            p = enode_cast(p, ty, 0, NULL);
    }

    return p;
}


/*
 *  inspects if a tree contains a function call
 */
static int hascall(const tree_t *p)
{
    assert(ir_cur);

    if (!p)
        return 0;
    if (op_generic(p->op) == OP_CALL)
        return 1;

    return hascall(p->kid[0]) || hascall(p->kid[1]);
}


/*
 *  parses arguments and calls tree_call();
 *  pointer() and value() apply to the function operand and each argument;
 *  ASSUMPTION: see TY_WIDEN() for assumptions
 */
tree_t *(tree_pcall)(tree_t *p)
{
    ty_t *ty;
    unsigned n = 0;
    tree_t *o, *arg = NULL, *r = NULL;
    ty_t *rty;
    void **proto;    /* ty_t */
    sym_t *t3 = NULL;
    const lmap_t *pos, *posp = NULL;

    assert(!p || p->type);
    assert(ty_voidtype);    /* ensures types initialized */
    assert(ir_cur);

    pos = clx_cpos;    /* ( */
    if (p) {
        p = enode_value(enode_pointer(p));
        ty = TY_UNQUAL(p->type);

        if (TY_ISPTR(ty) && TY_ISFUNC(ty->type))
            ty = ty->type;
        else {
            err_dpos(TREE_TW(p), ERR_EXPR_NOFUNC);
            p = NULL;
        }
    }
    clx_tc = clx_next();

    if (p) {
        rty = ty_freturn(ty);
        proto = (ty->u.f.oldstyle)? NULL: ty->u.f.proto;
        if (hascall(p))
            r = p;
        if (TY_ISSTRUNI(rty))
            t3 = sym_new(SYM_KTEMP, LEX_AUTO, TY_UNQUAL(rty), sym_scope);
    }

    if (clx_tc != ')')
        while (1) {
            tree_t *q;
            const lmap_t *pos = clx_cpos;
            if ((q = expr_asgn(0, 0, 1, NULL)) != NULL && p) {
                q = enode_value(enode_pointer(q));
                if (q->type->size == 0)
                    err_dpos(TREE_TW(q), ERR_EXPR_INCOMPARG, n+1, p, q->type);
                if (proto && *proto && *proto != ty_voidtype) {
                    ty_t *aty = enode_tcasgnty(T(*proto), q, TREE_TO(q), NULL);
                    if (!aty)
                        (void)(err_dpos(TREE_TW(q), ERR_EXPR_ARGNOTMATCH, n+1, p, q->type,
                                        T(*proto)) &&
                               err_dpos(ty->u.f.pos, ERR_PARSE_PREVDECL));
                    else if (aty != ty_voidtype)
                        q = enode_cast(q, aty, 1, TREE_TW(q));
                    if (TY_ISINTEGER(q->type) && q->type->size < ty_inttype->size)
                        q = enode_cast(q, ty_ipromote(q->type), 0, NULL);
                    proto++;
                } else {
                    if (!ty->u.f.oldstyle && !*proto && !posp)
                        posp = pos, o = p;    /* p may be set to NULL below */
                    else if (q->type->size > 0)
                        q = enode_cast(q, ty_apromote(q->type), 0, NULL);
                }
                if (!ir_cur->f.want_argb && TY_ISSTRUNI(q->type)) {
                    if (tree_iscallb(q))
                        q = addrof(q);
                    else {
                        sym_t *t1 = sym_new(SYM_KTEMP, LEX_AUTO, TY_UNQUAL(q->type), sym_scope);
                        tree_t *t = tree_addr(tree_id(t1, q->orgn->pos), ty_ptr(t1->type), 0,
                                              q->orgn->pos);
                        if (q->type->size == 0)
                            q = t;
                        else {
                            q = tree_asgnid(t1, q, q->orgn->pos);
                            q = tree_right(q, t, ty_ptr(t1->type), q->orgn->pos);
                        }
                    }
                }
            } else
                p = NULL;
            if (!q || q->type->size == 0)
                p = q = NULL;
            if (p) {
                if (hascall(q))
                    r = (r)? tree_right(r, q, ty_voidtype, q->orgn->pos): q;
                arg = tree_new(OP_ARG+OP_SFXW(q->type), q->type, q, arg, q->orgn->pos);
                if (n++ == TL_ARG_STD) {
                    pos = lmap_pin(pos);
                    (void)(err_dpos(pos, ERR_EXPR_MANYARG, p) &&
                           err_dpos(pos, ERR_EXPR_MANYARGSTD, (long)TL_ARG_STD));
                }
            }
            if (clx_tc != ',')
                break;
            clx_tc = clx_next();
            if (clx_xtracomma(')', "argument", 0))
                break;
        }
    if (posp)
        (void)(err_dpos(lmap_range(posp, clx_ppos), ERR_EXPR_EXTRAARG, o) &&
               err_dpos(ty->u.f.pos, ERR_PARSE_DECLHERE));
    posp = sset_expect(')', pos);
    if (!p)
        return NULL;

    if (proto && *proto && *proto != ty_voidtype)
        (void)(err_dpos((posp)? posp: lmap_after(clx_ppos), ERR_EXPR_INSUFFARG, p) &&
               err_dpos(ty->u.f.pos, ERR_PARSE_DECLHERE));

    if (r)
        arg = tree_right(r, arg, ty_voidtype, r->orgn->pos);

    p = call(p, rty, arg, t3, tree_npos(TREE_TL(p), pos, clx_ppos));
    if (TY_ISSTRUNI(rty) && rty->size == 0)
        err_dpos(TREE_TW(p), ERR_EXPR_RETINCOMP, rty);
    return p;
}


/*
 *  constructs a tree that accesses to a given member;
 *  ASSUMPTION: all pointers are uniform (same representation)
 */
static tree_t *field(tree_t *p, const char *name, tree_pos_t *tpos)
{
    sym_field_t *q;
    ty_t *ty, *uty;

    assert(p);
    assert(p->type);
    assert(name);
    assert(tpos);
    assert(ty_ptruinttype);    /* ensures types initialized */

    ty = p->type;
    if (TY_ISPTR(ty)) {
        ty = ty_deref(ty);
        assert(ty);
    }
    uty = TY_UNQUAL(ty);
    assert(TY_ISSTRUNI(uty));

    if ((q = ty_fieldref(name, uty)) != NULL) {
        uty = ty_qualc(ty->op & TY_CONSVOL, q->type);
        if (!TY_ISARRAY(uty))
            uty = ty_ptr(uty);
        {
            tree_t *r = p;
            p = tree_add(OP_ADD, tree_uconst(xiu(q->offset), ty_ptruinttype, tpos), p, uty, tpos);
            assert(!OP_ISADDR(r->op) || OP_ISADDR(p->op));
#ifdef NDEBUG
            UNUSED(r);
#endif    /* NDEBUG */
        }
        if (q->lsb) {
            p = tree_new(OP_FIELD, uty->type, tree_indir(p, NULL, 0, tpos), NULL, tpos);
            p->u.field = q;
        } else if (!TY_ISARRAY(q->type))
            p = tree_indir(p, NULL, 0, tpos);
    } else {
        if (uty->size == 0)
            err_dtpos(tpos, p, NULL, ERR_EXPR_DEREFINCOMP, uty);
        else
            err_dpos(clx_cpos, ERR_EXPR_UNKNOWNMEM, uty, name, "");
        p = NULL;
    }

    return p;
}


/*
 *  constructs a member-referencing tree;
 *  pointer() and value() apply to the left;
 *  ASSUMPTION: struct/union values are carried in temporary objects
 */
tree_t *(tree_dot)(int op, tree_t *p)
{
    tree_pos_t *tpos;
    const lmap_t *pos = clx_cpos;

    assert(op == '.' || op == LEX_DEREF);

    clx_tc = clx_next();

    if (clx_tc == LEX_ID) {
        tpos = tree_npos(TREE_NL(p), pos, clx_cpos);
        if (p) {
            p = enode_value(enode_pointer(p));
            if (op == '.') {
                if (TY_ISSTRUNI(p->type)) {
                    tree_t *q = addrof(p);
                    unsigned f = q->f.nlval;
                    ty_t *uqty = TY_UNQUAL(q->type);
                    if (!(TY_ISPTR(uqty) && TY_ISSTRUNI(uqty->type))) {
                        assert(err_count() > 0);
                        p = NULL;
                    } else {
                        p = field(q, clx_tok, tpos);
                        q = tree_rightkid(q);
                        if (p && (f || (OP_ISADDR(q->op) && q->u.sym->f.temporary))) {
                            p->f.nlval = 1;    /* for diagnostic by tree_right() */
                            p = tree_right(NULL, p, p->type, tpos);
                            p->f.nlval = 1;    /* for consistency */
                        }
                    }
                } else {
                    err_dtpos(tpos, p, NULL, ERR_EXPR_NOSTRUCT, p->type);
                    p = NULL;
                }
            } else {
                ty_t *upty = TY_UNQUAL(p->type);
                if (TY_ISPTR(upty) && TY_ISSTRUNI(upty->type))
                    p = field(p, clx_tok, tpos);
                else {
                    err_dtpos(tpos, p, NULL, ERR_EXPR_NOSTRUCTP, p->type);
                    p = NULL;
                }
            }
        }
        clx_tc = clx_next();
    } else if (p) {
        err_dpos(lmap_after(clx_ppos), ERR_EXPR_NOMEMBER);
        p = NULL;
    }

    return p;
}


/*
 *  constructs a constant tree with a signed integer
 */
tree_t *(tree_sconst)(sx_t n, ty_t *ty, tree_pos_t *tpos)
{
    tree_t *p = tree_new(OP_CNST+op_sfx(ty), ty, NULL, NULL, tpos);

    assert(ty_inttype);    /* ensures types initialized */

    switch(TY_RMQENUM(ty)->op) {
        case TY_INT:
            p->u.v.s = SYM_CROPSI(n);
            break;
        case TY_LONG:
            p->u.v.s = SYM_CROPSL(n);
            break;
#ifdef SUPPORT_LL
        case TY_LLONG:
            p->u.v.s = SYM_CROPSLL(n);
            break;
#endif    /* SUPPORT_LL */
        default:
            assert(!"invalid type operator -- should never reach here");
            break;
    }

    return p;
}


/*
 *  constructs a constant tree with an unsigned integer
 */
tree_t *(tree_uconst)(ux_t n, ty_t *ty, tree_pos_t *tpos)
{
    tree_t *p = tree_new(OP_CNST+op_sfx(ty), ty, NULL, NULL, tpos);

    assert(ty_unsignedtype);    /* ensures types initialized */

    switch(TY_RMQENUM(ty)->op) {
        case TY_UNSIGNED:
            p->u.v.u = SYM_CROPUI(n);
            break;
        case TY_ULONG:
            p->u.v.u = SYM_CROPUL(n);
            break;
#ifdef SUPPORT_LL
        case TY_ULLONG:
            p->u.v.u = SYM_CROPULL(n);
            break;
#endif    /* SUPPORT_LL */
        default:
            assert(!"invalid type operator -- should never reach here");
            break;
    }

    return p;
}


/*
 *  constructs a constant tree with a floating-point value
 */
tree_t *(tree_fpconst)(long double x, ty_t *ty, tree_pos_t *tpos)
{
    tree_t *p = tree_new(OP_CNST+op_sfx(ty), ty, NULL, NULL, tpos);

    switch(ty->op) {
        case TY_FLOAT:
            p->u.v.f = (float)x;
            break;
        case TY_DOUBLE:
            p->u.v.d = (double)x;
            break;
        case TY_LDOUBLE:
            p->u.v.ld = (long double)x;
            break;
        default:
            assert(!"invalid type operator -- should never reach here");
            break;
    }

    return p;
}


/*
 *  constructs an identifier tree
 */
tree_t *(tree_id)(sym_t *p, tree_pos_t *tpos)
{
    int op;
    tree_t *e;
    ty_t *ty;

    assert(p);
    assert(p->type);
    assert(!TY_ISUNKNOWN(p->type));
    assert(tpos);
    assert(ir_cur);

    ty = TY_UNQUAL(p->type);

    sym_ref(p, expr_refinc);

    if (p->scope == SYM_SGLOBAL || p->sclass == LEX_STATIC || p->sclass == LEX_EXTERN)
        op = op_addr(G);
    else if (p->scope == SYM_SPARAM) {
        op = op_addr(F);
        if (TY_ISSTRUNI(p->type) && !ir_cur->f.want_argb) {
            e = tree_new(op, ty_ptr(ty_ptr(p->type)), NULL, NULL, tpos);
            e->u.sym = p;
            e = tree_indir(tree_indir(e, NULL, 0, tpos), NULL, 0, tpos);
            return e;
        }
    } else
        op = op_addr(L);
    if (TY_ISARRAY(ty) || TY_ISFUNC(ty)) {
        e = tree_new(op, p->type, NULL, NULL, tpos);
        e->u.sym = p;
    } else {
        e = tree_new(op, ty_ptr(p->type), NULL, NULL, tpos);
        e->u.sym = p;
        e = tree_indir(e, NULL, 0, tpos);
    }

    return e;
}


/*
 *  finds the original tree for retyped trees
 */
tree_t *(tree_untype)(tree_t *p)
{
    if (p)
        while (p->kid[2])
            p = p->kid[2];

    return p;
}


#define S (1U << 4)    /* scan mode */
#define A (1U << 3)    /* left operand of ASGN */
#define I (1U << 2)    /* operand of INDIR */
#define P (1U << 1)    /* follows pointer operands */
#define V (1U << 0)    /* void expression context */

#define ADDRARGB(p) (op_generic((p)->op) == OP_ADDRF && TY_ISSTRUNI((p)->u.sym->type))

/*
 *  warns meaningless or errorneous references to symbols
 */
void (tree_chkref)(tree_t *p, unsigned f)
{
    tree_t *l, *r;

    assert(p);
    assert(ir_cur);

    p->f.checked = 1;    /* to avoid cycle;
                            tree_chkref() invoked initially with orgn */
    l = (!p->kid[0])? NULL: (p->kid[0]->orgn->f.checked)? p->kid[0]: p->kid[0]->orgn;
    r = (!p->kid[1])? NULL: (p->kid[1]->orgn->f.checked)? p->kid[1]: p->kid[1]->orgn;

    switch(op_generic(p->op)) {
        case OP_ASGN:    /* sets left's AP, clears right's IPV */
            assert(l);
            assert(r);
            assert(TY_ISPTR(l->type) || l->op == OP_FIELD);
            tree_chkref(r, f & ~(I|P|V));    /* right first to detect self-assignment */
            if (l->op == OP_FIELD) {
                assert(l->kid[0]);
                assert(op_generic(l->kid[0]->orgn->op) == OP_INDIR);
                p = l->kid[0]->orgn;
                l = p->kid[0]->orgn;
            }
            tree_chkref(l, f | S);
            tree_chkref(l, f | (A|P));
            break;
        case OP_INDIR:    /* sets IP */
            assert(l);
            if (!ir_cur->f.want_argb && ADDRARGB(l)) {
                tree_chkref(l, f);    /* ignores OP_INDIR */
                break;
            }
            tree_chkref(l, f | S);
            tree_chkref(l, f | (I|P));
            break;
        case OP_ADDRG:
        case OP_ADDRF:
        case OP_ADDRL:
            {
                sym_t *q;
                q = p->u.sym;
                if ((f & (I|P)) == (I|P))    /* IP */
                    q->f.reference = 1;
                if ((f & (A|P)) == (A|P))    /* AP */
                    q->f.set |= 1;
                if (!(f & (S|P))) {    /* encompasses !A!I!P */
                    q->f.set |= 2;
                    if (!(f & V))    /* encompasses !A!I!P!V */
                        q->f.reference = 1;
                }
            }
            break;
        case OP_RIGHT:    /* clears left and set V */
            if (tree_iscallb(p))
                tree_chkref(l, f);
            else {
                if (l)
                    tree_chkref(l, V);
                if (r)
                    tree_chkref(r, f);
            }
            break;
        case OP_CALL:
            if (op_type(p->op) == OP_B) {
                assert(l);
                assert(r);
                tree_chkref(l, f & ~V);
                tree_chkref(r, f | (A|P));
                break;
            }
            goto other;
        case OP_ADD:
            if (!ir_cur->f.want_argb && op_generic(l->op) == OP_INDIR &&
                ADDRARGB(p->kid[0]->kid[0])) {
                tree_chkref(p->kid[0]->kid[0]->orgn, f);    /* ignores OP_ADD and OP_INDIR */
                break;
            }
            /* no break */
        other:
        default:
            if (l)
                tree_chkref(l, f & ~((!TY_ISPTR(l->type))? (P|V): V));
            if (r)
                tree_chkref(r, f & ~((!TY_ISPTR(r->type))? (P|V): V));
            break;
    }
}

#undef S
#undef A
#undef I
#undef P
#undef V

#undef ADDRARGB


/*
 *  warns an unused expression
 *  ASSUMPTION: access to volatile object may be elided
 */
int (tree_chkused)(tree_t *p)
{
    if (!p)
        return 0;

    if (p->f.rooted)
        return 1;
    p->f.rooted = 1;

    switch(op_generic(p->op)) {
        case OP_CNST:
        case OP_ADDRG:
        case OP_ADDRF:
        case OP_ADDRL:
            if (main_opt()->std != 1 || !simp_needconst)
                err_dpos(TREE_TW(p), ERR_EXPR_VALNOTUSED);
            break;
        case OP_ARG:
        case OP_RET:
        case OP_JMP:
            return 0;
        case OP_ASGN:
        case OP_CALL:
            if (!p->f.ecast)
                return 0;
            err_dpos(TREE_TW(p), ERR_EXPR_VALNOTUSED);
            break;
        case OP_INDIR:
            if (TY_ISVOID(p->type))
                break;
            else if (p->type->size == 0) {
                err_dpos(TREE_TW(p), ERR_EXPR_SKIPREF);
                return 1;
            } else {
                ty_t *ty = TY_UNQUAL(p->kid[0]->type);
                if (TY_ISPTR(ty) && (TY_ISVOLATILE(ty->type) || TY_HASVOLATILE(ty->type))) {
                    err_dpos(TREE_TW(p), ERR_EXPR_SKIPVOLREF);
                    return 1;
                }
            }
            /* no break */
        case OP_NEG:
        case OP_ADD:
        case OP_SUB:
        case OP_LSH:
        case OP_MOD:
        case OP_RSH:
        case OP_BAND:
        case OP_BCOM:
        case OP_BOR:
        case OP_BXOR:
        case OP_DIV:
        case OP_MUL:
        case OP_EQ:
        case OP_GE:
        case OP_GT:
        case OP_LE:
        case OP_LT:
        case OP_NE:
        case OP_AND:
        case OP_NOT:
        case OP_OR:
        case OP_FIELD:
        case OP_POS:
            err_dpos(TREE_TW(p), ERR_EXPR_VALNOTUSED);
            break;
        case OP_CVF:
        case OP_CVI:
        case OP_CVU:
        case OP_CVP:
            if (!p->f.ecast)
                return tree_chkused(p->kid[0]->orgn);
            err_dpos(TREE_TW(p), ERR_EXPR_VALNOTUSED);
            break;
        case OP_COND:
            {
                tree_t *r, *t;

                assert(p->kid[1]);
                assert(p->kid[1]->op == OP_RIGHT);

                if (p->f.ecast) {
                    err_dpos(TREE_TW(p), ERR_EXPR_VALNOTUSED);
                    break;
                } else if (p->f.cvfpu)
                    return tree_chkused(p->kid[0]->kid[0]->orgn);

                r = p->kid[1];
                t = (p->u.sym && r->kid[0] && op_generic(r->kid[0]->op) == OP_ASGN)?
                        r->kid[0]->kid[1]->orgn: r->kid[0]->orgn;
                if (!tree_chkused(t)) {
                    t = (p->u.sym && r->kid[1] && op_generic(r->kid[1]->op) == OP_ASGN)?
                            r->kid[1]->kid[1]->orgn: r->kid[1]->orgn;
                    return tree_chkused(t);
                }
            }
            break;
        case OP_RIGHT:
            if (TY_ISVOID(p->type) || tree_iscallb(p) ||
                (p->kid[0] && p->kid[0]->op == OP_RIGHT &&
                 tree_untype(p->kid[1]) == tree_untype(p->kid[0]->kid[0])))
                return 0;
            if (p->kid[0])
                tree_chkused(p->kid[0]->orgn);
            if (p->f.ecast) {
                err_dpos(TREE_TW(p), ERR_EXPR_VALNOTUSED);
                break;
            }
            return (p->kid[1])? tree_chkused(p->kid[1]->orgn): 0;
        default:
            assert("invalid operation code -- should never reach here");
            break;
    }

    return 1;
}


/*
 *  checks if a static initializer has invalid operations
 */
int (tree_chkinit)(const tree_t *p)
{
    if (!p)
        return 0;

    if (op_generic(p->op) == OP_CALL || op_generic(p->op) == OP_ASGN)
        return TREE_FADDR;    /* ADDR for npce;
                                 no ACE|ICE since these get involved with pointers */

    return tree_chkinit(p->kid[0]) | tree_chkinit(p->kid[1]);
}


#ifndef NDEBUG
/*
 *  makes up a unique id for tree/dag nodes
 */
int (tree_pnodeid)(const void *p)
{
    static sz_t asize;

    int i;

    if (!parr) {
        asize = 0;    /* abandons allocated array */
        pid = 0;
    }
    if (asize >= pid) {
        void *old = parr;
        asize += 100;
        parr = ARENA_ALLOC(strg_func, asize * sizeof(*parr));
        memcpy(parr, old, pid * sizeof(*parr));
    }

    parr[pid].node = p;
    i = 0;
    while (parr[i].node != p)
        i++;
    if (i == pid)
        parr[pid++].printed = 0;

    return i;
}


/*
 *  checks if a tree/dag node has already been printed
 */
int *(tree_printed)(int id)
{
    assert(id >= 0 && id < pid);

    return &(parr[id].printed);
}


/*
 *  recursively prints a tree for debugging
 */
static void printtree(const tree_t *p, FILE *fp, int lev)
{
    int i;

    assert(fp);

    if (!p || *tree_printed(i=tree_pnodeid(p)))
        return;

    *(tree_printed(i)) = 1;
    fprintf(fp, "#% 4d ", i);
    for (i = 0; i < lev; i++)
        putc(' ', fp);
    fprintf(fp, (p->f.paren)? "(%s) %s": "%s %s", op_name(p->op), ty_outtype(p->type, 0));
    if (ty_hastypedef(p->type))
        fprintf(fp, " (%s)", ty_outtype(p->type, 1));
    if ((op_generic(p->op) == OP_CNST || op_generic(p->op) == OP_ADDRG) && p->f.npce)
        fputs(" N", fp);
    for (i = 0; i < NELEM(p->kid)-1; i++)
        if (p->kid[i])
            fprintf(fp, " #%d", tree_pnodeid(p->kid[i]));
    if (p->op == OP_FIELD) {
        assert(p->u.field);
        fprintf(fp, " %s %d...%d", p->u.field->name,
            (int)(SYM_FLDSIZE(p->u.field)+SYM_FLDRIGHT(p->u.field)), SYM_FLDRIGHT(p->u.field));
    } else if (op_generic(p->op) == OP_CNST)
        fprintf(fp, " %s", sym_vtoa(p->type, p->u.v));
    else if (p->u.sym)
        fprintf(fp, " %s", p->u.sym->name);
    if (p->node)
        fprintf(fp, " node=%p", (void *)p->node);
    putc('\n', fp);
    for (i = 0; i < NELEM(p->kid)-1; i++)
        printtree(p->kid[i], fp, lev+1);
}


/*
 *  starts a new array for printing tree/dags
 */
void (tree_printnew)(void)
{
    parr = NULL;
}


/*
 *  prints a tree for debugging
 */
void (tree_print)(const tree_t *p, FILE *fp)
{
    tree_printnew();
    printtree(p, fp, 0);
}
#endif    /* !NDEBUG */

/* end of tree.c */
