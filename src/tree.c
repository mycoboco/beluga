/*
 *  tree handling
 */

#include <stddef.h>        /* NULL */
#include <stdio.h>         /* sprintf */
#include <string.h>        /* strlen */
#include <cbl/arena.h>     /* arena_t, ARENA_CALLOC, ARENA_ALLOC */
#include <cbl/assert.h>    /* assert */
#ifndef NDEBUG
#include <stddef.h>        /* size_t */
#include <stdio.h>         /* FILE, fprintf, putc */
#include <string.h>        /* memcpy */
#endif    /* !NDEBUG */

#include "common.h"
#include "enode.h"
#include "err.h"
#include "expr.h"
#include "ir.h"
#include "lex.h"
#include "op.h"
#include "simp.h"
#include "strg.h"
#include "sym.h"
#include "ty.h"
#include "tree.h"

#define T(p) ((ty_t *)(p))    /* shorthand for cast to ty_t * */


/* tokens to tree-generating functions */
tree_t *(*tree_optree_s[])() = {
#define xx(a, b, c, d, e, f, g, h) e,
#define yy(a, b, c, d, e, f, g, h) e,
#include "xtoken.h"
};

/* tokens to tree opreators */
int tree_oper[] = {
#define xx(a, b, c, d, e, f, g, h) d,
#define yy(a, b, c, d, e, f, g, h) d,
#include "xtoken.h"
};


/* arena in which trees generated */
static arena_t **where = &strg_stmt;

#ifndef NDEBUG
/* # of used nodes in parr */
static int pid;

/* information for printing a tree */
static struct {
    const void *node;    /* for tree_t * and dag_node_t * */
    int printed;
} *parr;
#endif    /* !NDEBUG */


/*
 *  constructs a new tree
 */
tree_t *(tree_new_s)(int op, ty_t *ty, tree_t *l, tree_t *r)
{
    tree_t *p;

    assert(ty);
    assert(where);

    p = ARENA_CALLOC(*where, sizeof(*p), 1);
    p->op = op;
    p->type = ty;
    p->kid[0] = l;
    p->kid[1] = r;
    p->pos = *err_getppos();
    p->f.experr = ((l)? l->f.experr: 0) | ((r)? r->f.experr: 0);

    return p;
}


/*
 *  constructs a tree for an expression in a given arena
 */
tree_t *(tree_texpr)(tree_t *(*f)(int, int), int tok, arena_t *a)
{
    arena_t **save = where;
    tree_t *p;

    assert(f);
    assert(a);

    where = &a;
    p = f(tok, 0);
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


#define HANDLECHILD(i, lr)                                                  \
    if (p->u.sym && r->kid[i] && op_generic(r->kid[i]->op) == OP_ASGN) {    \
        t##lr = r->kid[i]->kid[1];                                          \
        r->kid[i] = tree_root_s(r->kid[i]->kid[1], &one);                   \
    } else {                                                                \
        t##lr = r->kid[i],                                                  \
        r->kid[i] = tree_root_s(r->kid[i], &one);                           \
    }

/*
 *  extracts a sub-tree with a side effect;
 *  ASSUMPTION: access to volatile object may be elided
 */
tree_t *(tree_root_s)(tree_t *p, int *pi)
{
    tree_t *l, *r;
    int f, issued = 0, one = 1;

    assert(ty_voidtype);    /* ensures types initialized */

    if (!p || p->f.rooted)
        return p;
    if (!pi)
        pi = &issued;
    f = *pi;
    if (p->f.experr && !main_opt()->_verbose_experr)
        *pi = 2;

    switch(op_generic(p->op)) {
        case OP_CNST:
        case OP_ADDRG:
        case OP_ADDRF:
        case OP_ADDRL:
            if (main_opt()->std != 1 || !simp_needconst) {
                if (!*pi)
                    err_issuep(&p->pos, ERR_EXPR_VALNOTUSED), *pi = 1;
                p = NULL;
            }
            break;
        case OP_ASGN:
        case OP_CALL:
            if ((p->f.ecast || p->f.omitop) && !*pi)
                err_issuep(&p->pos, ERR_EXPR_VALNOTUSED), *pi = 1;
            break;
        case OP_ARG:
        case OP_RET:
        case OP_JMP:
            break;
        case OP_INDIR:
            if (TY_ISVOID(p->type))
                *pi = 1;
            else if (p->type->size == 0 && !*pi)
                err_issuep(&p->pos, ERR_EXPR_SKIPREF), *pi = 1;
            else {
                ty_t *ty = TY_UNQUAL(p->kid[0]->type);
                if (TY_ISPTR(ty) && (TY_ISVOLATILE(ty->type) || TY_HASVOLATILE(ty->type)) && !*pi)
                err_issuep(&p->pos, ERR_EXPR_SKIPVOLREF), *pi = 1;
            }
            /* no break */
        case OP_NEG:
        case OP_BCOM:
        case OP_NOT:
        case OP_FIELD:
            if (!*pi)
                err_issuep(&p->pos, ERR_EXPR_VALNOTUSED), *pi = 1;
            p = tree_root_s(p->kid[0], pi);
            break;
        case OP_CVF:
        case OP_CVI:
        case OP_CVU:
        case OP_CVP:
            if (p->f.ecast && !*pi)
                err_issuep(&p->pos, ERR_EXPR_VALNOTUSED), *pi = 1;
            p = tree_root_s(p->kid[0], pi);
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
            if (!*pi)
                err_issuep(&p->pos, ERR_EXPR_VALNOTUSED), *pi = 1;
            l = tree_root_s(p->kid[0], pi);
            r = tree_root_s(p->kid[1], pi);
            p = (l && r)? tree_new_s(OP_RIGHT, p->type, l, r):
                (l)? l: r;    /* tree_right_s() not used here */
            break;
        case OP_AND:
        case OP_OR:
            if (!*pi)
                err_issuep(&p->pos, ERR_EXPR_VALNOTUSED), *pi = 1;
            if ((p->kid[1]=tree_root_s(p->kid[1], pi)) == NULL)
                p = tree_root_s(p->kid[0], pi);
            break;
        case OP_COND:
            {
                tree_t *tl, *tr;

                assert(p->kid[1]);
                assert(p->kid[1]->op == OP_RIGHT);

                if ((p->f.ecast || p->f.omitop) && !*pi)
                    err_issuep(&p->pos, ERR_EXPR_VALNOTUSED), *pi = 1;
                if (p->f.cvfpu) {
                    p = tree_root_s(p->kid[0]->kid[0], pi);
                    break;
                }
                r = p->kid[1];
                HANDLECHILD(0, l);
                HANDLECHILD(1, r);
                if (!r->kid[0] && !r->kid[1]) {
                    if (!*pi && p->u.sym)
                        err_issuep(&p->pos, ERR_EXPR_VALNOTUSED), *pi = 1;
                    p = tree_root_s(p->kid[0], pi);
                } else {
                    p->u.sym = NULL;    /* sym no longer used */
                    if (!*pi) {
                        int issued = 0;
                        if (r->kid[0])
                            tree_root_s(tl, &issued);
                        if (r->kid[1] && !issued)
                            tree_root_s(tr, NULL);
                    }
                }
            }
            break;
        case OP_RIGHT:
            if (p->kid[0] && p->kid[0]->op == OP_RIGHT &&
                tree_untype(p->kid[1]) == tree_untype(p->kid[0]->kid[0])) {
                p = p->kid[0]->kid[1];
                break;
            }
            if (p->type->t.type == ty_voidtype && !p->kid[0] && p->kid[1]) {
                if ((p->kid[1]=tree_root_s(p->kid[1], &one)) == NULL)
                    p = NULL;
                break;
            }
            if (tree_iscallb(p))
                *pi = 1;
            one = *pi & ~1U;
            p->kid[0] = tree_root_s(p->kid[0], &one);
            p->kid[1] = tree_root_s(p->kid[1], pi);
            p = (!p->kid[0])? p->kid[1]: (!p->kid[1])? p->kid[0]: p;
            break;
        default:
            assert("invalid operation code -- should never reach here");
            break;
    }

    if (p)
        p->f.rooted = !f;
    return p;
}

#undef HANDLECHILD


/*
 *  constructs a tree with a type/pos changed
 */
tree_t *(tree_retype_s)(tree_t *p, ty_t *ty)
{
    tree_t *q;

    assert(p);

    /* always copy for resetting pos;
       kid[2] keeps original tree for diagnostics */
    q = tree_new_s(p->op, (ty)? ty: p->type, p->kid[0], p->kid[1]);
    q->f = p->f;
    q->u = p->u;
    q->kid[2] = p;

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
        sprintf(name, "`%s'", f->u.sym->name);
        return name;
    }

    return "a function";
}


/*
 *  constructs a tree to access a struct/union object from another tree;
 *  ASSUMPTION: struct/union value is stored in a temporary object
 */
tree_t *(tree_addrof_s)(tree_t *p)
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
                    if (!t1)
                        goto nosym;
                    q->u.sym = NULL;
                    q = tree_id_s(t1);
                }
                /* no break */
            case OP_INDIR:
                if (p == q)
                    return p->kid[0];
                q = q->kid[0];
                q = tree_right_s(p, q, q->type);
                goto ret;
            nosym:
            default:
                err_experr_s(p->f.experr, ERR_EXPR_NEEDOBJ);
                q = tree_retype_s(enode_value_s(p), NULL);
                goto ret;
        }

    ret:
        return ENODE_MRGEXPERRT(p, q);
}


/*
 *  constructs a RIGHT tree;
 *  root() applies to the left;
 *  pointer() applies to the right;
 *  left child is NULL if only one child exists
 */
tree_t *(tree_right_s)(tree_t *l, tree_t *r, ty_t *ty)
{
    assert(l || r);

    if (!r) {
        r = l;
        l = NULL;
    }
    if (l)
        l = enode_pointer_s(l);
    r = enode_pointer_s(r);

    if (!ty)
        ty = r->type;
    ty = TY_UNQUAL(ty);

    /* cannot remove type check to preserve rvalue-ness */
    if (!l && r->op == OP_RIGHT && ty_same(ty, r->type))
        return tree_retype_s(r, ty);
    return tree_new_s(OP_RIGHT, ty, l, r);
}


/*
 *  constructs an ASGN tree;
 *  value() applies to the right;
 *  pointer() applies to both;
 *  ASSUMPTION: bit-field is singed or unsigned int;
 *  ASSUMPTION: overflow of left shift is silently ignored on the target
 */
static tree_t *asgn_s(int op, tree_t *l, tree_t *r, ty_t *ty, int force)
{
    ty_t *aty;

    assert(op == OP_ASGN || op == OP_INCR || op == OP_DECR || op == OP_ADD || op == OP_SUB ||
           op == OP_MUL || op == OP_DIV || op == OP_MOD || op == OP_LSH || op == OP_RSH ||
           op == OP_BAND || op == OP_BXOR || op == OP_BOR);
    assert(l);
    assert(r);
    assert(ty_voidtype);    /* ensures types initialized */

    l = enode_pointer_s(l);
    r = enode_value_s(enode_pointer_s(r));

    if (!ty)
        ty = enode_tcasgn_s(l, r);
    if (ty == ty_voidtype) {
        err_experr_s(l->f.experr | r->f.experr, ERR_EXPR_ASGNINCOMP);
        if (main_opt()->_verbose_experr)
            ty = l->type;
        else
            return enode_setexperr(tree_right_s(NULL, l, l->type));
    } else if (ty) {
        if (TY_ISSCALAR(ty))
            r = enode_cast_s(r, ty, ENODE_FCHKOVF);
    } else if (TY_ISARRAY(l->type) || op == OP_INCR || op == OP_DECR) {
        assert(!(op == OP_INCR || op == OP_DECR) || err_count() > 0);
        ty = l->type;
    } else
        return enode_typeerr_s(OP_ASGN, l, r);
    ty = TY_UNQUAL(ty);

    if (l->op != OP_FIELD)
        l = tree_addr_s(l, NULL, 0);
    aty = l->type;
    if (TY_ISPTR(aty))
        aty = TY_UNQUAL(aty)->type;
    if (!force && (TY_ISCONST(aty) || TY_HASCONST(aty)))
        err_issue_s(ERR_EXPR_ASGNCONST, (OP_ISADDR(l->op))? l->u.sym: NULL, " location");
    if (l->op == OP_FIELD) {
        int n = TG_CHAR_BIT*l->u.field->type->size - SYM_FLDSIZE(l->u.field);
        if (n > 0) {
            if (TY_UNQUAL(l->u.field->type) == ty_unsignedtype)
                r = tree_bit_s(OP_BAND,
                               r, tree_uconst_s(SYM_FLDMASK(l->u.field), ty_unsignedtype),
                               ty_unsignedtype);
            else {
                assert(TY_UNQUAL(l->u.field->type) == ty_inttype);
                if (op_generic(r->op) == OP_CNST) {
                    if (!SYM_INFIELD(r->u.v.li, l->u.field))
                        err_issue_s(ERR_EXPR_BIGFLD);
                    r = tree_sconst_s(sym_sextend(r->u.v.li, l->u.field), ty_inttype);
                } else
                    r = tree_sha_s(OP_RSH,
                                   tree_sha_s(OP_LSH, r, tree_sconst_s(n, ty_inttype), ty_inttype),
                                   tree_sconst_s(n, ty_inttype),
                                   ty_inttype);
            }
        }
    }

    if (TY_ISSTRUNI(ty) && OP_ISADDR(l->op) && tree_iscallb(r))
        return tree_right_s(    /* tree_call() cannot be used here */
                   tree_new_s(OP_CALL+OP_B, ty, r->kid[0]->kid[0], l),
                   tree_id_s(l->u.sym), ty);
    return tree_new_s(OP_ASGN+op_sfxs(ty), ty, l, r);
}


/*
 *  constructs an ASGN tree
 */
tree_t *(tree_asgn_s)(int op, tree_t *l, tree_t *r, ty_t *ty)
{
    return asgn_s(op, l, r, ty, 0);
}


/*
 *  constructs an ASGN tree ignoring const qualification
 */
tree_t *(tree_asgnf_s)(int op, tree_t *l, tree_t *r, ty_t *ty)
{
    return asgn_s(op, l, r, ty, 1);
}


/*
 *  constructs a tree to assign an expression to a symbol;
 *  const on the symbol ignored in constructing a tree
 */
tree_t *(tree_asgnid_s)(sym_t *p, tree_t *e)
{
    assert(p);
    assert(e);
    assert(p->type);

    return (TY_ISARRAY(p->type))?
               tree_new_s(OP_ASGN+OP_B, p->type,
                          tree_id_s(p),
                          tree_new_s(OP_INDIR+OP_B, e->type, e, NULL)):
               asgn_s(OP_ASGN, tree_id_s(p), e, NULL, 1);
}


/*
 *  constructs a tree for compound assignments;
 *  a preset result type ty is not used thus not delivered;
 *  note that the first parameter is not op code but token code
 */
tree_t *(tree_casgn_s)(int tc, tree_t *v, tree_t *e)
{
    assert(v);
    assert(e);

    return asgn_s(tree_oper[tc],
                  v,
                  tree_optree_s[tc](tree_oper[tc], v, e, NULL),
                  NULL, 0);
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
tree_t *(tree_cond_s)(tree_t *e, tree_t *l, tree_t *r, ty_t *ty)
{
    tree_t *p;
    sym_t *t1;

    assert(e);
    assert(l);
    assert(r);
    assert(ty_inttype);    /* ensures types initialized */

    e = enode_chkcond(OP_COND, e, "first operand of ");
    l = enode_value_s(enode_pointer_s(l));
    r = enode_value_s(enode_pointer_s(r));

    if (!ty) {
        ty = enode_tccond_s(l, r);
        if (!ty)
            return enode_typeerr_s(OP_COND, l, r);
    }
    assert(!TY_ISQUAL(ty));

    if (op_generic(e->op) == OP_CNST) {
        int npce = e->f.npce;
        switch(op_type(e->op)) {
            case OP_P:
                e = enode_cast_s((e->u.v.tp)? l: r, ty, 0);
                npce |= (TREE_FADDR|TREE_FACE|TREE_FICE);
                break;
            case OP_F:
                e = (enode_cast_s(e, ty_ldoubletype, 0)->u.v.ld)? l: (p=r, r=l, l=p);
                npce |= TREE_FICE;
                goto branch;
            default:
                e = (enode_cast_s(e, ty_ulongtype, 0)->u.v.ul)? l: (p=r, r=l, l=p);
                /* no break */
            branch:
                e = enode_cast_s(e, ty, 0);
                r->f.npce &= (TREE_FACE|TREE_FICE);    /* masks COMMA|ADDR */
                if (main_opt()->std == 1)
                    npce |= tree_chkinit(r);    /* ADDR */
                SETNPCE(l);
                SETNPCE(r);
                break;
        }
        if (op_generic(e->op) == OP_CNST || op_generic(e->op) == OP_ADDRG)
            e->f.npce = npce;
        return e;
    }
    if (ty->t.type != ty_voidtype && ty->size > 0) {
        t1 = sym_new(SYM_KTEMP, LEX_REGISTER, ty, sym_scope);
        l = tree_asgnid_s(t1, l);
        r = tree_asgnid_s(t1, r);
    } else
        t1 = NULL;
    p = tree_new_s(OP_COND, ty, enode_cond_s(e),    /* tree_right() cannot be used here */
                                tree_new_s(OP_RIGHT, ty, l, r));
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
tree_t *(tree_and_s)(int op, tree_t *l, tree_t *r, ty_t *ty)
{
    assert(op == OP_AND || op == OP_OR);
    assert(l);
    assert(r);

    l = enode_chkcond(op, l, "left operand of ");
    r = enode_chkcond(op, r, "right operand of ");

    if (!ty) {
        ty = enode_tcand(l, r);
        if (!ty)
            return enode_typeerr_s(op, l, r);
    }
    assert(!TY_ISQUAL(ty));

    return simp_tree_s(op, ty, enode_cond_s(l), enode_cond_s(r));
}


/*
 *  constructs a BAND, BOR, BXOR or MOD tree;
 *  pointer() and value() apply to both
 */
tree_t *(tree_bit_s)(int op, tree_t *l, tree_t *r, ty_t *ty)
{
    ty_t *uty;

    assert(op == OP_BAND || op == OP_BOR || op == OP_BXOR || op == OP_MOD);
    assert(l);
    assert(r);

    l = enode_value_s(enode_pointer_s(l));
    r = enode_value_s(enode_pointer_s(r));

    if (!ty) {
        ty = enode_tcbit(l, r);
        if (!ty)
            return enode_typeerr_s(op, l, r);
    }
    assert(!TY_ISQUAL(ty));

    l = enode_cast_s(l, ty, 0);
    r = enode_cast_s(r, ty, 0);
    if (op != OP_MOD) {
        uty = ty_ucounter(l->type);
        l = enode_cast_s(l, uty, 0);
        r = enode_cast_s(r, uty, 0);
    }

    if (op == OP_MOD)
        return simp_tree_s(op, ty, l, r);
    return enode_cast_s(simp_tree_s(op, uty, l, r), ty, 0);
}


/*
 *  constructs a LE, GE, LT, GT, EQ or NE tree;
 *  pointer() and value() apply to both;
 *  ASSUMPTION: pointers are converted to integer for comparison
 */
tree_t *(tree_cmp_s)(int op, tree_t *l, tree_t *r, ty_t *ty)
{
    int npce = 0;

    assert(op == OP_LE || op == OP_GE || op == OP_LT || op == OP_GT || op == OP_EQ || op == OP_NE);
    assert(l);
    assert(r);
    assert(ty_voidtype);    /* ensures types initialized */

    l = enode_value_s(enode_pointer_s(l));
    r = enode_value_s(enode_pointer_s(r));

    if (!ty) {
        ty = (op == OP_EQ || op == OP_NE)? enode_tceq(l, r): enode_tccmp(l, r);
        if (!ty)
            return enode_typeerr_s(op, l, r);
    }

    ty = TY_RMQENUM(ty);    /* ty->op used below */
    if (ty == ty_voidtype)
        return tree_cmp_s(op, r, l, NULL);
    if (ty == ty_voidptype) {
        ty = ty_ptruinttype;
        npce = (TREE_FADDR|TREE_FACE|TREE_FICE);    /* catches comparing NPCs */
    } else if (TY_ISFP(ty))
        npce = TREE_FICE;
    l = enode_cast_s(l, ty, 0);
    r = enode_cast_s(r, ty, 0);

    l = simp_tree_s(op + ty->op, ty_inttype, l, r);
    if (op_generic(l->op) == OP_CNST && npce)
        l->f.npce |= npce;

    return l;
}


/*
 *  constructs a RSH or LSH tree;
 *  pointer() and value() apply to both;
 *  ASSUMPTION: RSH always performs arithmetic shift on the target
 */
tree_t *(tree_sha_s)(int op, tree_t *l, tree_t *r, ty_t *ty)
{
    assert(op == OP_RSH || op == OP_LSH);
    assert(l);
    assert(r);
    assert(ty_inttype);    /* ensures types initialized */

    l = enode_value_s(enode_pointer_s(l));
    r = enode_value_s(enode_pointer_s(r));

    if (!ty) {
        ty = enode_tcsh(l, r);
        if (!ty)
            return enode_typeerr_s(op, l, r);
    }
    assert(!TY_ISQUAL(ty));

    l = enode_cast_s(l, ty, 0);
    r = enode_cast_s(r, ty_inttype, 0);

    return simp_tree_s(op, ty, l, r);
}


/*
 *  constructs a RSH or LSH tree;
 *  unsigned shift used when logical shift turned on for RSH
 */
tree_t *(tree_sh_s)(int op, tree_t *l, tree_t *r, ty_t *ty)
{
    ty_t *lty;

    assert(l);

    lty = ty_ipromote(l->type);

    if (!TY_ISINTEGER(lty))
        return tree_sha_s(op, l, r, ty);
    if (main_opt()->logicshift && op == OP_RSH && !TY_ISUNSIGN(lty)) {
        /* logical shift has no chance to warn so copied from warnnegrsh() in simp.c */
        if (op_generic(l->op) == OP_CNST && l->u.v.li < 0)
            err_issue_s(ERR_EXPR_RSHIFTNEG);
        l = enode_cast_s(l, ty_ucounter(lty), 0);
    }
    return enode_cast_s(tree_sha_s(op, l, r, ty), lty, 0);
}


/*
 *  constructs an ADD tree;
 *  pointer() and value() apply to both;
 *  ASSUMPTION: pointer arithmetic can be implemented using integers;
 *  ASSUMPTION: all pointers are uniform (same representation)
 */
tree_t *(tree_add_s)(int op, tree_t *l, tree_t *r, ty_t *ty)
{
    ty_t *gty = ty;

    assert(op == OP_ADD || op == OP_INCR || op == OP_SUBS);
    assert(l);
    assert(r);
    assert(ty_voidtype);    /* ensures types initialized */

    l = enode_value_s(enode_pointer_s(l));
    r = enode_value_s(enode_pointer_s(r));

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
                enode_typeerr_s(op, (op_optype(l->op) == OP_CNST+OP_I && l->u.v.li == 1)? r: l,
                                NULL):
                enode_typeerr_s(op, l, r);
    }

    if (ty == ty_voidtype)
        return tree_add_s(op, r, l, NULL);
    else if (TY_ISARITH(ty)) {
        l = enode_cast_s(l, ty, 0);
        r = enode_cast_s(r, ty, 0);
    } else {
        if (!gty) {    /* do math only when no given type */
            long n;    /* signed because of negative indices */
            n = ty->type->size;
            if (n == 0)
                err_issuep(&r->pos, ERR_EXPR_UNKNOWNSIZE, ty->type);
            else if (l->f.npce & TREE_FICE)
                op = 0;
            l = enode_cast_s(l, ty_ipromote(l->type), 0);
            if (n > 1)
                l = tree_mul_s(OP_MUL, tree_sconst_s(n, ty_ptrsinttype), l, NULL);
        }
        l = simp_tree_s(OP_ADD + TY_POINTER, ty, l, r);
        if (op_generic(l->op) == OP_ADDRG && !op)
            l->f.npce |= TREE_FADDR;
        return l;
    }

    l = simp_tree_s(OP_ADD, ty, l, r);
    if (op_generic(l->op) == OP_CNST && TY_ISFP(ty))
        l->f.npce |= TREE_FICE;

    return l;
}


/*
 *  construct a SUB tree;
 *  pointer() and value() apply to both;
 *  ASSUMPTION: pointer arithmetic can be implemented using integers;
 *  ASSUMPTION: signed integers are compatible with unsigned ones on the target;
 *  ASSUMPTION: all pointers are uniform (same representation)
 */
tree_t *(tree_sub_s)(int op, tree_t *l, tree_t *r, ty_t *ty)
{
    long n;

    assert(op == OP_SUB || op == OP_DECR);
    assert(l);
    assert(r);
    assert(!ty || TY_ISARITH(ty));    /* given type should be arithmetic if any */
    assert(ty_ptrsinttype);               /* ensures types initialized */

    l = enode_value_s(enode_pointer_s(l));
    r = enode_value_s(enode_pointer_s(r));

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
                enode_typeerr_s(op, (op_optype(l->op) == OP_CNST+OP_I && l->u.v.li == 1)? r: l,
                                NULL):
                enode_typeerr_s(op, l, r);
    }

    if (TY_ISPTR(ty)) {
        ty = TY_UNQUAL(ty);
        n = ty->type->size;
        if (n == 0)
            err_issuep(&l->pos, ERR_EXPR_UNKNOWNSIZE, ty->type);
        else if (r->f.npce & TREE_FICE)
            op = 0;
        r = enode_cast_s(r, ty_ipromote(r->type), 0);
        if (n > 1)
            r = tree_mul_s(OP_MUL, tree_sconst_s(n, ty_ptrsinttype), r, NULL);
        r = simp_tree_s(OP_SUB + TY_POINTER, ty, l, r);
        if (op_generic(r->op) == OP_ADDRG && !op)
            r->f.npce |= TREE_FADDR;
        return r;
    } else if (ty == ty_voidtype) {
        ty = TY_UNQUAL(l->type);
        n = ty->type->size;
        if (n == 0) {
            err_issuep(&l->pos, ERR_EXPR_UNKNOWNSIZE, ty->type);
            n = 1;    /* to avoid divide by 0 */
        } else
            ty = NULL;
        l = simp_tree_s(OP_SUB, ty_ptruinttype, enode_cast_s(l, ty_ptruinttype, 0),
                                                enode_cast_s(r, ty_ptruinttype, 0));
        l = enode_cast_s(
                tree_mul_s(OP_DIV, enode_cast_s(l, ty_ptrsinttype, 0),
                                   tree_sconst_s(n, ty_ptrsinttype),
                           ty_ptrsinttype),
                ty_ptrdifftype, 0);
        if (op_generic(l->op) == OP_CNST && !ty)
            l->f.npce |= TREE_FADDR;
        return l;
    } else {
        l = enode_cast_s(l, ty, 0);
        r = enode_cast_s(r, ty, 0);
    }

    l = simp_tree_s(OP_SUB, ty, l, r);
    if (op_generic(l->op) == OP_CNST && TY_ISFP(ty))
        l->f.npce |= TREE_FICE;

    return l;
}


/*
 *  construct a MUL or DIV tree;
 *  pointer() and value() apply to both
 */
tree_t *(tree_mul_s)(int op, tree_t *l, tree_t *r, ty_t *ty)
{
    assert(op == OP_MUL || op == OP_DIV);
    assert(l);
    assert(r);

    l = enode_value_s(enode_pointer_s(l));
    r = enode_value_s(enode_pointer_s(r));

    if (!ty) {
        ty = enode_tcmul(l, r);
        if (!ty)
            return enode_typeerr_s(op, l, r);
    }
    assert(!TY_ISQUAL(ty));

    l = enode_cast_s(l, ty, 0);
    r = enode_cast_s(r, ty, 0);

    l = simp_tree_s(op, ty, l, r);
    if (op_generic(l->op) == OP_CNST && TY_ISFP(ty))
        l->f.npce |= TREE_FICE;

    return l;
}


/*
 *  constructs a tree for the unary + operation;
 *  pointer() and value() apply to the operand;
 */
tree_t *(tree_pos_s)(tree_t *p, ty_t *ty)
{
    tree_t *q = p;

    assert(p);

    p = enode_value_s(enode_pointer_s(p));

    if (!ty) {
        ty = enode_tcposneg(p);
        if (!ty)
            return enode_typeerr_s(OP_ADD, p, NULL);
    }

    p = enode_cast_s(p, ty, 0);
    if (op_generic(p->op) == OP_INDIR)
        p = tree_right_s(NULL, p, NULL);

    if (q == p)
        p = tree_retype_s(p, NULL);
    if (op_generic(p->op) == OP_CNST && TY_ISFP(ty))
        p->f.npce |= TREE_FICE;
    p->f.omitop = 1;

    return p;
}


/*
 *  constructs a NEG tree;
 *  pointer() and value() apply to the operand;
 *  ASSUMPTION: (unsigned)(-(signed)value) implements negation of unsigned
 */
tree_t *(tree_neg_s)(tree_t *p, ty_t *ty)
{
    assert(p);

    p = enode_value_s(enode_pointer_s(p));

    if (!ty) {
        ty = enode_tcposneg(p);
        if (!ty)
            return enode_typeerr_s(OP_SUB, p, NULL);
    }

    p = enode_cast_s(p, ty, 0);
    if (TY_ISUNSIGN(p->type)) {
        ty_t *sty = ty_scounter(p->type);
        err_issue_s(ERR_EXPR_NEGUNSIGNED);
        err_mute();    /* ERR_EXPR_NEGUNSIGNED is enough */
        p = simp_tree_s(OP_NEG, sty, enode_cast_s(p, sty, 0), NULL);
        p = enode_cast_s(p, ty, 0);
        err_unmute();
    } else {
        p = simp_tree_s(OP_NEG, p->type, p, NULL);
        if (op_generic(p->op) == OP_CNST && TY_ISFP(ty))
            p->f.npce |= TREE_FICE;
    }

    return p;
}


/*
 *  constructs a BCOM tree;
 *  pointer() and value() apply to the operand
 */
tree_t *(tree_bcom_s)(tree_t *p, ty_t *ty)
{
    assert(p);

    p = enode_value_s(enode_pointer_s(p));

    if (!ty) {
        ty = enode_tcbcom(p);
        if (!ty)
            return enode_typeerr_s(OP_BCOM, p, NULL);
    }

    p = simp_tree_s(OP_BCOM, ty, enode_cast_s(p, ty, 0), NULL);

    return p;
}


/*
 *  constructs a NOT tree;
 *  pointer() and cond() applies to the operand
 */
tree_t *(tree_not_s)(tree_t *p, ty_t *ty)
{
    assert(p);

    p = enode_pointer_s(p);

    if (!ty) {
        ty = enode_tcnot(p);
        if (!ty)
            return enode_typeerr_s(OP_NOT, p, NULL);
    }

    p = simp_tree_s(OP_NOT, ty, enode_cond_s(p), NULL);

    return p;
}


/*
 *  constructs an INDIR tree;
 *  pointer() and value() apply to the operand;
 *  TODO: warn of breaking anti-aliasing rules;
 *  TODO: warn of referencing uninitialized objects
 */
tree_t *(tree_indir_s)(tree_t *p, ty_t *ty, int explicit)
{
    int err = 0;

    assert(p);

    p = enode_value_s(enode_pointer_s(p));

    if (!ty) {
        ty = enode_tcindir_s(p, &err);
        assert(ty);
    }

    if (TY_ISFUNC(ty) || TY_ISARRAY(ty))
        p = tree_retype_s(p, ty);
    else {
        p = tree_new_s(OP_INDIR+op_sfxs(ty), ty, p, NULL);
        p->f.eindir = explicit;
    }

    return (err)? enode_setexperr(p): p;
}


/*
 *  constructs an address(lvalue) tree
 */
tree_t *(tree_addr_s)(tree_t *p, ty_t *ty, int explicit)
{
    assert(p);
    assert(ty_voidtype);    /* ensures types initialized */

    if (!ty) {
        ty = enode_tcaddr(p);
        if (!ty) {
            err_experr_s(p->f.experr, ERR_EXPR_NEEDLVALUE);
            return enode_setexperr(tree_right_s(NULL, enode_value_s(p), ty_ptr(p->type)));
        } else if (explicit && TY_ISVOID(ty))
            err_issue_s((ty == ty_voidtype)? ERR_EXPR_VOIDLVALUE1: ERR_EXPR_VOIDLVALUE2);
    }

    if (TY_ISPTR(ty) && (TY_ISFUNC(ty->type) || TY_ISARRAY(ty->type)))
        return tree_retype_s(p, ty);

    return tree_retype_s(p->kid[0], NULL);
}


/*
 *  constructs a CALL tree after arguments parsed;
 *  ASSUMPTION: pointers can be returned as an integer;
 *  ASSUMPTION: see TY_WIDEN() for additional assumptions
 */
tree_t *(tree_call_s)(tree_t *f, ty_t *ty, tree_t *args, sym_t *t3)
{
    tree_t *p;

    assert(f);
    assert(ty);
    assert(ty_ptruinttype);    /* ensures types initialized */

    if (args)
        f = tree_right_s(args, f, f->type);
    if (TY_ISSTRUNI(ty)) {
        assert(t3);
        p = tree_right_s(tree_new_s(OP_CALL+OP_B, ty,
                                    f, tree_addr_s(tree_id_s(t3),
                                                   ty_ptr(t3->type), 0)),
                         tree_id_s(t3), ty);
    } else {
        ty_t *rty = TY_UNQUAL(ty);
        if (TY_ISPTR(ty))
            rty = ty_ptruinttype;
        p = tree_new_s(OP_CALL+OP_SFXW(rty), ty_ipromote(ty), f, NULL);
        if (TY_ISPTR(ty) || p->type->size > ty->size)
            p = enode_cast_s(p, ty, 0);
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
    tree_t *arg = NULL, *r = NULL;
    ty_t *rty;
    void **proto;    /* ty_t */
    sym_t *t3 = NULL;

    assert(p);
    assert(p->type);
    assert(ty_voidtype);    /* ensures types initialized */
    assert(ir_cur);

    err_entersite(&lex_cpos);    /* enters with ( */
    p = enode_value_s(enode_pointer_s(p));
    ty = TY_UNQUAL(p->type);

    if (TY_ISPTR(ty) && TY_ISFUNC(ty->type))
        ty = ty->type;
    else {
        err_entersite(&p->pos);    /* enters with function expression */
        err_experr_s(p->f.experr, ERR_EXPR_NOFUNC);
        ty = ty_func_s(ty_voidtype, NULL, 1);
        p = enode_setexperr(tree_retype_s(p, ty_ptr(ty)));
        err_exitsite();    /* exits from function expression */
    }
    lex_tc = lex_next();

    rty = ty_freturn(ty);
    proto = (ty->u.f.oldstyle)? NULL: ty->u.f.proto;
    if (hascall(p))
        r = p;
    if (TY_ISSTRUNI(rty)) {
        t3 = sym_new(SYM_KTEMP, LEX_AUTO, TY_UNQUAL(rty), sym_scope);
        if (rty->size == 0)
            err_issue_s(ERR_EXPR_RETINCOMP, rty);
    }

    if (lex_tc != ')')
        while (1) {
            tree_t *q;
            err_entersite(&lex_cpos);    /* enters with argument */
            q = enode_value_s(enode_pointer_s(expr_asgn(0, 0, 1)));
            if (q->type->size == 0)
                err_issue_s(ERR_EXPR_INCOMPARG, n+1, p, q->type);
            if (proto && *proto && *proto != ty_voidtype) {
                ty_t *aty = enode_tcasgnty_s(T(*proto), q);
                if (!aty)
                    err_issue_s(ERR_EXPR_ARGNOTMATCH, n+1, p, q->type, T(*proto));
                else if (aty != ty_voidtype)
                    q = enode_cast_s(q, aty, ENODE_FCHKOVF);
                if (TY_ISINTEGER(q->type) && q->type->size < ty_inttype->size)
                    q = enode_cast_s(q, ty_ipromote(q->type), 0);
                proto++;
            } else {
                if (!ty->u.f.oldstyle && !*proto)
                    err_issue_s(ERR_EXPR_EXTRAARG, p);
                else if (q->type->size > 0)
                    q = enode_cast_s(q, ty_apromote(q->type), 0);
            }
            if (!ir_cur->f.want_argb && TY_ISSTRUNI(q->type)) {
                if (tree_iscallb(q))
                    q = tree_addrof_s(q);
                else {
                    sym_t *t1 = sym_new(SYM_KTEMP, LEX_AUTO, TY_UNQUAL(q->type), sym_scope);
                    tree_t *t = tree_addr_s(tree_id_s(t1), ty_ptr(t1->type), 0);
                    if (q->type->size == 0)
                        q = t;
                    else {
                        q = tree_asgnid_s(t1, q);
                        q = tree_right_s(q, t, ty_ptr(t1->type));
                    }
                }
            }
            if (q->type->size == 0)
                q->type = ty_inttype;
            if (hascall(q))
                r = (r)? tree_right_s(r, q, ty_voidtype): q;
            arg = tree_new_s(OP_ARG+OP_SFXW(q->type), q->type, q, arg);
            if (n++ == TL_ARG_STD) {
                err_issue_s(ERR_EXPR_MANYARG, p);
                err_issue_s(ERR_EXPR_MANYARGSTD, (long)TL_ARG_STD);
            }
            err_exitsite();    /* exits from argument */
            if (lex_tc != ',')
                break;
            lex_tc = lex_next();
            if (lex_extracomma(')', "argument", 0))
                break;
        }
    err_expect(')');
    if (proto && *proto && *proto != ty_voidtype)
        err_issue_s(ERR_EXPR_INSUFFARG, p);
    if (r)
        arg = tree_right_s(r, arg, ty_voidtype);

    p = tree_call_s(p, rty, arg, t3);
    err_exitsite();    /* exits from ( */
    return p;
}


/*
 *  constructs a tree that accesses to a given member;
 *  ASSUMPTION: all pointers are uniform (same representation)
 */
static tree_t *field_s(tree_t *p, const char *name)
{
    sym_field_t *q;
    ty_t *ty, *uty;

    assert(p);
    assert(p->type);
    assert(name);
    assert(ty_ptruinttype);    /* ensures types initialized */

    ty = p->type;
    if (TY_ISPTR(ty))
        ty = ty_deref_s(ty, NULL);
    uty = TY_UNQUAL(ty);
    assert(TY_ISSTRUNI(uty));

    if ((q = ty_fieldref(name, uty)) != NULL) {
        uty = ty_qualc(ty->op & TY_CONSVOL, q->type);
        if (!TY_ISARRAY(uty))
            uty = ty_ptr(uty);
        {
            tree_t *r = p;
            p = tree_add_s(OP_ADD, tree_uconst_s(q->offset, ty_ptruinttype), p, uty);
            assert(!OP_ISADDR(r->op) || OP_ISADDR(p->op));
#ifdef NDEBUG
            UNUSED(r);
#endif    /* NDEBUG */
        }
        if (q->lsb) {
            p = tree_new_s(OP_FIELD, uty->type, tree_indir_s(p, NULL, 0), NULL);
            p->u.field = q;
        } else if (!TY_ISARRAY(q->type))
            p = tree_indir_s(p, NULL, 0);
    } else {
        err_issue_s(ERR_EXPR_UNKNOWNMEM, name, "");
        p = tree_indir_s(tree_retype_s(p, ty_ptr(ty_inttype)), ty_inttype, 0);
    }

    return p;
}


/*
 *  constructs a member-referencing tree;
 *  pointer() and value() apply to the left;
 *  ASSUMPTION: struct/union value is carried in a temporary object
 */
tree_t *(tree_dot_s)(int op, tree_t *p)
{
    assert(op == '.' || op == LEX_DEREF);
    assert(p);

    lex_tc = lex_next();
    p = enode_value_s(enode_pointer_s(p));

    if (lex_tc == LEX_ID) {
        if (op == '.') {
            if (TY_ISSTRUNI(p->type)) {
                tree_t *q = tree_addrof_s(p);
                ty_t *uqty = TY_UNQUAL(q->type);
                if (!(TY_ISPTR(uqty) && TY_ISSTRUNI(uqty->type))) {
                    assert(err_count() > 0);
                    q = tree_right_s(NULL, q, p->type);
                }
                p = field_s(q, lex_tok);
                q = tree_rightkid(q);
                if (OP_ISADDR(q->op) && q->u.sym->f.temporary)
                    p = tree_right_s(NULL, p, p->type);
            } else {
                err_experr_s(p->f.experr, ERR_EXPR_NOSTRUCT1, p->type);
                enode_setexperr(p);
            }
        } else {
            ty_t *upty = TY_UNQUAL(p->type);
            if (TY_ISPTR(upty) && TY_ISSTRUNI(upty->type))
                p = field_s(p, lex_tok);
            else {
                err_experr_s(p->f.experr, ERR_EXPR_NOSTRUCT2, p->type);
                enode_setexperr(p);
            }
        }
        lex_tc = lex_next();
    } else
        err_issuex(0, ERR_EXPR_NOMEMBER);

    return p;
}


/*
 *  constructs a constant tree with a signed integer
 */
tree_t *(tree_sconst_s)(long n, ty_t *ty)
{
    tree_t *p = tree_new_s(OP_CNST+op_sfx(ty), ty, NULL, NULL);

    assert(ty_inttype);    /* ensures types initialized */

    switch(TY_RMQENUM(ty)->op) {
        case TY_INT:
            p->u.v.li = SYM_CROPSI(n);
            break;
        case TY_LONG:
            p->u.v.li = SYM_CROPSL(n);
            break;
        default:
            assert(!"invalid type operator -- should never reach here");
            break;
    }

    return p;
}


/*
 *  constructs a constant tree with an unsigned integer
 */
tree_t *(tree_uconst_s)(unsigned long n, ty_t *ty)
{
    tree_t *p = tree_new_s(OP_CNST+op_sfx(ty), ty, NULL, NULL);

    assert(ty_unsignedtype);    /* ensures types initialized */

    switch(TY_RMQENUM(ty)->op) {
        case TY_UNSIGNED:
            p->u.v.ul = SYM_CROPUI(n);
            break;
        case TY_ULONG:
            p->u.v.ul = SYM_CROPUL(n);
            break;
        default:
            assert(!"invalid type operator -- should never reach here");
            break;
    }

    return p;
}


/*
 *  constructs a constant tree with a floating-point value
 */
tree_t *(tree_fpconst_s)(long double x, ty_t *ty)
{
    tree_t *p = tree_new_s(OP_CNST+op_sfx(ty), ty, NULL, NULL);

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
tree_t *(tree_id_s)(sym_t *p)
{
    int op;
    tree_t *e;
    ty_t *ty;

    assert(p);
    assert(p->type);
    assert(ir_cur);

    ty = TY_UNQUAL(p->type);

    sym_ref(p, expr_refinc);

    if (p->scope == SYM_SGLOBAL || p->sclass == LEX_STATIC || p->sclass == LEX_EXTERN)
        op = op_addr(G);
    else if (p->scope == SYM_SPARAM) {
        op = op_addr(F);
        if (TY_ISSTRUNI(p->type) && !ir_cur->f.want_argb) {
            e = tree_new_s(op, ty_ptr(ty_ptr(p->type)), NULL, NULL);
            e->u.sym = p;
            e = tree_indir_s(tree_indir_s(e, NULL, 0), NULL, 0);
            if (p->f.undecl)
                enode_setexperr(e);
            return e;
        }
    } else
        op = op_addr(L);
    if (TY_ISARRAY(ty) || TY_ISFUNC(ty)) {
        e = tree_new_s(op, p->type, NULL, NULL);
        e->u.sym = p;
    } else {
        e = tree_new_s(op, ty_ptr(p->type), NULL, NULL);
        e->u.sym = p;
        e = tree_indir_s(e, NULL, 0);
    }

    if (p->f.undecl)
        enode_setexperr(e);
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


#define S (1 << 4)    /* scan mode */
#define A (1 << 3)    /* left operand of ASGN */
#define I (1 << 2)    /* operand of INDIR */
#define P (1 << 1)    /* follows pointer operands */
#define V (1 << 0)    /* void expression context */

#define ADDRARGB(p) (op_generic((p)->op) == OP_ADDRF && TY_ISSTRUNI((p)->u.sym->type))

/*
 *  warns meaningless or errorneous references to symbols
 */
void (tree_chkref)(tree_t *p, unsigned f)
{
    assert(ir_cur);

    if (!p || (p->f.experr && !main_opt()->_verbose_experr))
        return;

    switch(op_generic(p->op)) {
        case OP_ASGN:    /* sets left's AP, clears right's IPV */
            assert(p->kid[0]);
            assert(p->kid[1]);
            assert(TY_ISPTR(p->kid[0]->type) || p->kid[0]->op == OP_FIELD);
            tree_chkref(p->kid[1], f & ~(I|P|V));    /* right first to detect self-assignment */
            if (p->kid[0]->op == OP_FIELD) {
                assert(p->kid[0]->kid[0]);
                assert(op_generic(p->kid[0]->kid[0]->op) == OP_INDIR);
                p = p->kid[0]->kid[0];
            }
            tree_chkref(p->kid[0], f | S);
            tree_chkref(p->kid[0], f | (A|P));
            break;
        case OP_INDIR:    /* sets IP */
            assert(p->kid[0]);
            if (!ir_cur->f.want_argb && ADDRARGB(p->kid[0])) {
                tree_chkref(p->kid[0], f);    /* ignores OP_INDIR */
                break;
            }
            tree_chkref(p->kid[0], f | S);
            tree_chkref(p->kid[0], f | (I|P));
            break;
        case OP_ADDRG:
        case OP_ADDRF:
        case OP_ADDRL:
            {
                sym_t *q;
                p = tree_untype(simp_basetree(NULL, p));
                q = p->u.sym;
                if ((f & (I|P)) == (I|P))    /* IP */
                    q->f.reference = 1;
                if ((f & (A|P)) == (A|P)) {    /* AP */
                    q->f.set |= 1;
                    if (!(f & V))    /* AP!V */
                        q->f.reference = 1;
                }
                if (!(f & (S|P))) {    /* encompasses !A!I!P */
                    q->f.set |= 2;
                    if (!(f & V))    /* encompasses !A!I!P!V */
                        q->f.reference = 1;
                }
            }
            break;
        case OP_RIGHT:    /* clears left and set V */
            if (p->kid[0])
                tree_chkref(p->kid[0], V);
            if (p->kid[1])
                tree_chkref(p->kid[1], f);
            break;
        case OP_CALL:
            if (op_type(p->op) == OP_B) {
                assert(p->kid[0]);
                assert(p->kid[1]);
                assert(OP_ISADDR(p->kid[1]->op));
                tree_chkref(p->kid[0], f & ~V);
                tree_chkref(p->kid[1], f | (A | P));
                break;
            }
            goto other;
        case OP_ADD:
            if (!ir_cur->f.want_argb && op_generic(p->kid[0]->op) == OP_INDIR &&
                ADDRARGB(p->kid[0]->kid[0])) {
                tree_chkref(p->kid[0]->kid[0], f);    /* ignores OP_ADD and OP_INDIR */
                break;
            }
            /* no break */
        other:
        default:
            if (p->kid[0])
                tree_chkref(p->kid[0], f & ~((!TY_ISPTR(p->kid[0]->type))? (P|V): V));
            if (p->kid[1])
                tree_chkref(p->kid[1], f & ~((!TY_ISPTR(p->kid[1]->type))? (P|V): V));
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
    static size_t asize;

    int i;

    if (!parr) {
        asize = 0;    /* abondons allocated array */
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
    fprintf(fp, "%s %s", op_name(p->op), ty_outtype(p->type, 0));
    if (ty_hastypedef(p->type))
        fprintf(fp, " (%s)", ty_outtype(p->type, 1));
    fprintf(fp, " !%d", p->f.experr);
    if ((op_generic(p->op) == OP_CNST || op_generic(p->op) == OP_ADDRG) && p->f.npce)
        fputc('N', fp);
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
