/*
 *  expression dag
 */

#include <stddef.h>       /* NULL */
#include <string.h>       /* memset, memcpy */
#include <cbl/arena.h>    /* ARENA_CALLOC, ARENA_FREE */
#include <cbl/assert.h>   /* assert */
#ifndef NDEBUG
#include <stdio.h>        /* FILE, fprintf, putc */
#endif    /* !NDEBUG */

#include "common.h"
#include "decl.h"
#include "enode.h"
#include "err.h"
#include "init.h"
#include "ir.h"
#include "lex.h"
#include "main.h"
#include "op.h"
#include "stmt.h"
#include "strg.h"
#include "sym.h"
#include "tree.h"
#include "ty.h"
#include "dag.h"


/* visits kids for undagging */
#define VISITKID() do {                                    \
                       p->kid[0] = visit(p->kid[0], 0);    \
                       p->kid[1] = visit(p->kid[1], 0);    \
                   } while(0)

#define S(p) ((sym_t *)(p))    /* shorthand for cast to sym_t * */


int dag_nodecount;    /* # of nodes in hash buckets */


/* hash buckets for reuse of dag nodes */
struct dag_t {
    dag_node_t node;        /* dag node; not pointer */
    struct dag_t *hlink;    /* hash link */
};


static struct dag_t *bucket[16];    /* hash buckets */
static dag_node_t *forest;          /* dag forest */
static tree_t *firstarg;            /* hidden first argument (!want_callb) */
static int depth;                   /* nesting depth of && and || */
static dag_node_t **tail;           /* where to add nodes while undagging */


/*
 *  generates a pure dag node
 */
static struct dag_t *dagnode(int op, dag_node_t *l, dag_node_t *r, sym_t *sym)
{
    struct dag_t *p;

    p = ARENA_CALLOC(strg_func, 1, sizeof(*p));
    p->node.op = op;
    if ((p->node.kid[0] = l) != NULL)
        l->count++;
    if ((p->node.kid[1] = r) != NULL)
        r->count++;
    p->node.sym[0] = sym;

    return p;
}


/*
 *  returns a dag node with inspection on hash buckets
 */
static dag_node_t *node(int op, dag_node_t *l, dag_node_t *r, sym_t *sym)
{
    int i;
    struct dag_t *p;

    i = (op_index(op) ^ ((unsigned)sym >> 3)) & (NELEM(bucket)-1);
    for (p = bucket[i]; p; p = p->hlink)
        if (p->node.op == op && p->node.sym[0] == sym && p->node.kid[0] == l &&
            p->node.kid[1] == r)
            return &p->node;
    p = dagnode(op, l, r, sym);
    p->hlink = bucket[i];
    bucket[i] = p;
    dag_nodecount++;

    return &p->node;
}


/*
 *  returns a dag node without inspection on hash buckets
 */
dag_node_t *(dag_newnode)(int op, dag_node_t *l, dag_node_t *r, sym_t *sym)
{
    return &(dagnode(op, l, r, sym)->node);
}


/*
 *  kills rvalue nodes in hash buckets
 */
static void kill(const sym_t *p)
{
    int i;
    struct dag_t **q;

    assert(p);

    for (i = 0; i < NELEM(bucket); i++)
        for (q = &bucket[i]; *q; )
            if (op_generic((*q)->node.op) == OP_INDIR &&
                (!OP_ISADDR((*q)->node.kid[0]->op) || (*q)->node.kid[0]->sym[0] == p)) {
                *q = (*q)->hlink;
                dag_nodecount--;
            } else
                q = &(*q)->hlink;
}


/*
 *  clears hash buckets
 */
static void reset(void)
{
    if (dag_nodecount > 0)
        memset(bucket, 0, sizeof(bucket));
    dag_nodecount = 0;
}


/*
 *  lists a dag node as a root in the forest
 */
static void list(dag_node_t *p)
{
    assert(p);

    if (!p->link) {
        if (forest) {
            p->link = forest->link;
            forest->link = p;
        } else
            p->link = p;
        forest = p;
    }
}


/*
 *  lists a label node in the forest
 */
static void labelnode(int lab)
{
    assert(lab > 0);

    if (forest && forest->op == OP_LABELV)
        stmt_eqlabel(sym_findlabel(lab), forest->sym[0]);
    else
        list(dag_newnode(OP_LABELV, NULL, NULL, sym_findlabel(lab)));
    reset();
}


/*
 *  removes a dag node from the forest
 */
static void unlist(void)
{
    dag_node_t *p;

    assert(forest);
    assert(forest != forest->link);

    p = forest->link;
    while (p->link != forest)
        p = p->link;
    p->link = forest->link;
    forest = p;
}


/*
 *  converts an out-of-line constant to a symbol containing it
 */
static tree_t *cvtconst(tree_t *p)
{
    sym_t *q;
    tree_t *e;

    assert(p);
    assert(!TY_ISARRAY(p->type));

    q = sym_findconst(p->type, p->u.v);
    if (!q->u.c.loc)
        q->u.c.loc = sym_new(SYM_KGEN, LEX_STATIC, p->type, SYM_SGLOBAL);
    q->u.c.loc->f.outofline = 1;
    err_entersite(&p->pos);
    e = tree_id_s(q->u.c.loc);
    err_exitsite();

    return e;
}


/*
 *  constructs an extended basic block by building dags from trees;
 *  ASSUMPTION: overflow of left shift is silently ignored on the target;
 *  ASSUMPTION: unsigned long is compatible with signed one on the host;
 *  ASSUMPTION: volatile reference alone constitutes no side effect
 */
dag_node_t *(dag_listnode)(tree_t *tp, int tlab, int flab)
{
    tree_t *u;
    dag_node_t *p = NULL, *l, *r;

    assert((!tlab ^ !flab) || (!tlab && !flab));
    assert(ty_inttype);    /* ensures types initialized */
    assert(ir_cur);

#if 0    /* disabled for performance; see DAG_LISTNODE() */
    if (!tp)
        return NULL;
#else
    assert(tp);
#endif    /* disabled */

    u = tree_untype(tp);    /* only untyped trees have node set */
    if (u->node) {
        u->node->usecse = 1;
        return u->node;
    }

    err_entersite(&tp->pos);
    switch(op_generic(tp->op)) {
        case OP_CNST:
            {
                tree_t *t;
                ty_t *ty = TY_UNQUAL(tp->type);
                assert(ty->u.sym);
                if (tlab || flab) {
                    assert(ty->t.type == ty_inttype);
                    if (tlab && tp->u.v.ul != 0)
                        list(stmt_jump(tlab));
                    else if (flab && tp->u.v.ul == 0)
                        list(stmt_jump(flab));
                } else if (ty->u.sym->f.outofline) {
                    t = cvtconst(tp);
                    p = DAG_LISTNODE(t, 0, 0);
                } else
                    p = node(tp->op, NULL, NULL, sym_findconst(ty, tp->u.v));
            }
            break;
        case OP_ARG:
            assert(!tlab && !flab);
            if (ir_cur->f.left_to_right)
                DAG_LISTNODE(tp->kid[1], 0, 0);
            if (firstarg) {
                tree_t *arg = firstarg;
                firstarg = NULL;
                DAG_LISTNODE(arg, 0, 0);
            }
            l = DAG_LISTNODE(tp->kid[0], 0, 0);
            list(dag_newnode(tp->op, l, NULL, NULL));
            forest->sym[0] = sym_findint(tp->type->size);
            forest->sym[1] = sym_findint(tp->type->align);
            if (!ir_cur->f.left_to_right)
                DAG_LISTNODE(tp->kid[1], 0, 0);
            break;
        case OP_ASGN:
            assert(!tlab && !flab);
            if (tp->kid[0]->op == OP_FIELD) {
                tree_t *x = tp->kid[0]->kid[0];
                sym_field_t *f = tp->kid[0]->u.field;
                assert(op_generic(x->op) == OP_INDIR);
                l = dag_listnode(tree_addr_s(x, NULL, 0), 0, 0);
                if (SYM_FLDSIZE(f) < TG_CHAR_BIT*f->type->size) {
                    unsigned fmask = SYM_FLDMASK(f);
                    unsigned mask = fmask << SYM_FLDRIGHT(f);
                    tree_t *q = tp->kid[1];
                    if (op_generic(q->op) == OP_CNST && q->u.v.ul == 0)
                        q = tree_bit_s(OP_BAND, x, tree_uconst_s(~mask, ty_unsignedtype),
                                       ty_unsignedtype);
                    else if (op_generic(q->op) == OP_CNST && (q->u.v.ul & fmask) == fmask)
                        q = tree_bit_s(OP_BOR, x, tree_uconst_s(mask, ty_unsignedtype),
                                       ty_unsignedtype);
                    else {
                        DAG_LISTNODE(q, 0, 0);
                        q = tree_bit_s(OP_BOR,
                                tree_bit_s(OP_BAND,
                                    tree_indir_s(tree_addr_s(x, NULL, 0), NULL, 0),
                                    tree_uconst_s(~mask, ty_unsignedtype),
                                    ty_unsignedtype),
                                tree_bit_s(OP_BAND,
                                    tree_sh_s(OP_LSH,
                                        enode_cast_s(q, ty_unsignedtype, 0),
                                        tree_sconst_s(SYM_FLDRIGHT(f), ty_inttype),
                                        ty_unsignedtype),
                                    tree_uconst_s(mask, ty_unsignedtype),
                                    ty_unsignedtype),
                                ty_unsignedtype);
                    }
                    r = DAG_LISTNODE(q, 0, 0);
                } else
                    r = DAG_LISTNODE(tp->kid[1], 0, 0);
            } else {
                l = DAG_LISTNODE(tp->kid[0], 0, 0);
                r = DAG_LISTNODE(tp->kid[1], 0, 0);
            }
            list(dag_newnode(tp->op, l, r, NULL));
            assert(forest);
            forest->sym[0] = sym_findint(tp->kid[1]->type->size);
            forest->sym[1] = sym_findint(tp->kid[1]->type->align);
            if (OP_ISADDR(tp->kid[0]->op) && !tp->kid[0]->u.sym->f.computed)
                kill(tp->kid[0]->u.sym);
            else
                reset();
            p = DAG_LISTNODE(tp->kid[1], 0, 0);
            break;
        case OP_INDIR:
            {
                ty_t *ty = tp->kid[0]->type;
                assert(!tlab && !flab);
                l = DAG_LISTNODE(tp->kid[0], 0, 0);
                if (TY_ISPTR(ty))
                    ty = TY_UNQUAL(ty)->type;
                assert(l);
                if (TY_ISVOLATILE(ty) || TY_HASVOLATILE(ty))
                    p = dag_newnode(tp->op, l, NULL, NULL);
                else
                    p = node(tp->op, l, NULL, NULL);
            }
            break;
        case OP_CVF:    /* unary */
        case OP_CVI:
        case OP_CVU:
        case OP_CVP:
        case OP_NEG:
        case OP_BCOM:
            assert(!tlab && !flab);
            l = DAG_LISTNODE(tp->kid[0], 0, 0);
            p = node(tp->op, l, NULL, NULL);
            break;
        case OP_CALL:
            {
                tree_t *save = firstarg;
                assert(!tlab && !flab);
                firstarg = NULL;
                if (tp->op == OP_CALL+OP_B && !ir_cur->f.want_callb) {
                    tree_t *arg0;
                    arg0 = tree_new_s(OP_ARG+op_sfx(tp->kid[1]->type), tp->kid[1]->type,
                                      tp->kid[1], NULL);
                    if (ir_cur->f.left_to_right)
                        firstarg = arg0;
                    l = DAG_LISTNODE(tp->kid[0], 0, 0);
                    if (!ir_cur->f.left_to_right || firstarg) {
                        firstarg = NULL;
                        DAG_LISTNODE(arg0, 0, 0);
                    }
                    p = dag_newnode(OP_CALLV, l, NULL, NULL);
                } else {
                    l = DAG_LISTNODE(tp->kid[0], 0, 0);
                    r = DAG_LISTNODE(tp->kid[1], 0, 0);
                    p = dag_newnode(tp->op, l, r, NULL);
                }
                /* CALL's sym[1] carries function type to back-end */
                p->sym[1] = ARENA_CALLOC(strg_func, 1, sizeof(*p->sym[0]));
                p->sym[1]->type = tp->kid[0]->type->type;
                list(p);
                reset();
                decl_cfunc->u.f.ncall++;
                firstarg = save;
            }
            break;
        case OP_RET:
            assert(!tlab && !flab);
            l = DAG_LISTNODE(tp->kid[0], 0, 0);
            list(dag_newnode(tp->op, l, NULL, NULL));
            break;
        case OP_ADDRL:
            if (tp->u.sym->f.temporary)
                stmt_local(tp->u.sym);
            /* no break */
        case OP_ADDRG:
        case OP_ADDRF:
            assert(!tlab && !flab);
            p = node(tp->op, NULL, NULL, tp->u.sym);
            break;
        case OP_ADD:    /* binary */
        case OP_SUB:
        case OP_LSH:
        case OP_MOD:

        case OP_RSH:
        case OP_BAND:
        case OP_BOR:
        case OP_BXOR:
        case OP_DIV:
        case OP_MUL:
            assert(!tlab && !flab);
            l = DAG_LISTNODE(tp->kid[0], 0, 0);
            r = DAG_LISTNODE(tp->kid[1], 0, 0);
            p = node(tp->op, l, r, NULL);
            break;
        case OP_EQ:    /* conditional jump */
        case OP_GE:
        case OP_GT:
        case OP_LE:
        case OP_LT:
        case OP_NE:
            assert(!tp->u.sym);
            l = DAG_LISTNODE(tp->kid[0], 0, 0);
            r = DAG_LISTNODE(tp->kid[1], 0, 0);
            if (tlab)
                list(dag_newnode(tp->op, l, r, sym_findlabel(tlab)));
            else {
                int op;
                assert(flab);
                switch(op_generic(tp->op)) {
                    case OP_EQ:
                        op = OP_NE + op_tyscode(tp->op);
                        break;
                    case OP_NE:
                        op = OP_EQ + op_tyscode(tp->op);
                        break;
                    case OP_GT:
                        op = OP_LE + op_tyscode(tp->op);
                        break;
                    case OP_GE:
                        op = OP_LT + op_tyscode(tp->op);
                        break;
                    case OP_LT:
                        op = OP_GE + op_tyscode(tp->op);
                        break;
                    case OP_LE:
                        op = OP_GT + op_tyscode(tp->op);
                        break;
                    default:
                        assert(!"invalid operation code -- should never reach here");
                        break;
                }
                list(dag_newnode(op, l, r, sym_findlabel(flab)));
            }
            assert(forest);
            if (forest->sym[0])
                sym_ref(forest->sym[0], 1);
            break;
        case OP_JMP:
            assert(!tlab && !flab);
            assert(!tp->u.sym);
            assert(tp->kid[0]);
            l = DAG_LISTNODE(tp->kid[0], 0, 0);
            list(dag_newnode(OP_JMPV, l, NULL, NULL));
            reset();
            break;
        case OP_AND:
            if (depth++ == 0)
                reset();
            if (flab) {
                DAG_LISTNODE(tp->kid[0], 0, flab);
                DAG_LISTNODE(tp->kid[1], 0, flab);
            } else {
                DAG_LISTNODE(tp->kid[0], 0, flab=sym_genlab(1));
                DAG_LISTNODE(tp->kid[1], tlab, 0);
                labelnode(flab);
            }
            depth--;
            break;
        case OP_OR:
            if (depth++ == 0)
                reset();
            if (tlab) {
                DAG_LISTNODE(tp->kid[0], tlab, 0);
                DAG_LISTNODE(tp->kid[1], tlab, 0);
            } else {
                DAG_LISTNODE(tp->kid[0], tlab=sym_genlab(1), 0);
                DAG_LISTNODE(tp->kid[1], 0, flab);
                labelnode(tlab);
            }
            depth--;
            break;
        case OP_NOT:
            p = DAG_LISTNODE(tp->kid[0], flab, tlab);
            break;
        case OP_COND:
            {
                tree_t *q = tp->kid[1];
                assert(!tlab && !flab);
                assert(q && q->op == OP_RIGHT);
                if (tp->u.sym)
                    stmt_local(tp->u.sym);
                flab = sym_genlab(2);
                DAG_LISTNODE(tp->kid[0], 0, flab);
                reset();
                DAG_LISTNODE(q->kid[0], 0, 0);
                if (forest->op == OP_LABELV) {
                    stmt_eqlabel(forest->sym[0], sym_findlabel(flab+1));
                    unlist();
                }
                list(stmt_jump(flab + 1));
                labelnode(flab);
                DAG_LISTNODE(q->kid[1], 0, 0);
                labelnode(flab + 1);
                if (tp->u.sym)
                    p = dag_listnode(tree_id_s(tp->u.sym), 0, 0);
            }
            break;
        case OP_RIGHT:
            if (tp->kid[0] && tp->kid[1] && op_generic(tp->kid[1]->op) == OP_ASGN &&
                ((op_generic(tp->kid[0]->op) == OP_INDIR &&
                  tree_untype(tp->kid[0]->kid[0]) == tree_untype(tp->kid[1]->kid[0])) ||
                 (tp->kid[0]->op == OP_FIELD &&
                  tree_untype(tp->kid[0]) == tree_untype(tp->kid[1]->kid[0])))) {
                assert(!tlab && !flab);
                if (op_generic(tp->kid[0]->op) == OP_INDIR) {
                    p = DAG_LISTNODE(tp->kid[0], 0, 0);
                    list(p);
                    DAG_LISTNODE(tp->kid[1], 0, 0);
                } else {
                    assert(op_generic(tp->kid[0]->kid[0]->op) == OP_INDIR);
                    list(DAG_LISTNODE(tp->kid[0]->kid[0], 0, 0));
                    p = DAG_LISTNODE(tp->kid[0], 0, 0);
                    DAG_LISTNODE(tp->kid[1], 0, 0);
                }
            } else {
                assert(tp->kid[1]);
                DAG_LISTNODE(tp->kid[0], 0, 0);    /* may be empty */
                p = DAG_LISTNODE(tp->kid[1], tlab, flab);
            }
            break;
        case OP_FIELD:
            {
                tree_t *q;
                assert(!tlab && !flab);
                q = tree_sha_s(OP_RSH,
                        tree_sha_s(OP_LSH,
                                   tp->kid[0],
                                   tree_sconst_s(SYM_FLDLEFT(tp->u.field), ty_inttype),
                                   NULL),
                        tree_sconst_s(TG_CHAR_BIT*tp->type->size - SYM_FLDSIZE(tp->u.field),
                                      ty_inttype),
                        NULL);
                p = DAG_LISTNODE(q, 0, 0);
            }
            break;
        default:
            assert(!"invalid operation code -- should never reach here");
            break;
    }
    err_exitsite();

    return (u->node = p);
}


/*
 *  turns the forest into a Gen entry of the code list
 */
void (dag_walk)(tree_t *t, int tlab, int flab)
{
    DAG_LISTNODE(t, tlab, flab);
    if (forest) {
        stmt_new(STMT_GEN)->u.forest = forest->link;
        forest->link = NULL;
        forest = NULL;
    }
    reset();
    ARENA_FREE(strg_stmt);
}


/*
 *  finds the real label from a list of label synonyms
 */
static sym_t *equated(sym_t *p)
{
    sym_t *q;

    assert(p);

    for (q = p->u.l.equatedto; q; q = q->u.l.equatedto)
        assert(p != q);

    while (p->u.l.equatedto)
        p = p->u.l.equatedto;

    return p;
}


/*
 *  sets the destination labels of jumps to their real ones
 */
static void fixup(dag_node_t *p)
{
    for (; p; p = p->link)
        switch(op_generic(p->op)) {
            case OP_JMP:
                if (op_generic(p->kid[0]->op) == OP_ADDRG) {
                    assert(p->kid[0]->sym[0]);
                    p->kid[0]->sym[0] = equated(p->kid[0]->sym[0]);
                }
                break;
            case OP_LABEL:
                assert(p->sym[0] == equated(p->sym[0]));
                break;
            case OP_EQ:
            case OP_GE:
            case OP_GT:
            case OP_LE:
            case OP_LT:
            case OP_NE:
                assert(p->sym[0]);
                p->sym[0] = equated(p->sym[0]);
                break;
        }
}


/*
 *  constructs a dag node to refer to multiply referenced temporaries
 */
static dag_node_t *tmpnode(dag_node_t *p)
{
    sym_t *tmp;

    assert(p);
    assert(p->sym[2]);

    tmp = p->sym[2];
    p->count--;
    p = dag_newnode(OP_INDIR+op_sfxs(tmp->type), dag_newnode(op_addr(L), NULL, NULL, tmp), NULL,
                    NULL);
    p->count = 1;

    return p;
}


/*
 *  constructs an assignment node for multiply referenced temporaries
 */
static dag_node_t *asgnnode(sym_t *tmp, dag_node_t *p)
{
    assert(tmp);
    assert(p);

    p = dag_newnode(OP_ASGN+op_sfxs(tmp->type), dag_newnode(op_addr(L), NULL, NULL, tmp), p, NULL);
    p->sym[0] = sym_findint(tmp->type->size);
    p->sym[1] = sym_findint(tmp->type->align);

    return p;
}


/*
 *  visits nodes to replace multiply referenced nodes
 */
static dag_node_t *visit(dag_node_t *p, int listed)
{
    assert(ir_cur);

    if (p) {
        if (p->sym[2])
            p = tmpnode(p);
        else if ((p->count <= 1 && op_generic(p->op) != OP_CALL) ||
                 (p->count == 0 && op_generic(p->op) == OP_CALL))
            VISITKID();
        else if (op_generic(p->op) == OP_ADDRL || op_generic(p->op) == OP_ADDRF) {
            assert(!listed);
            p = dag_newnode(p->op, NULL, NULL, p->sym[0]);
            p->count = 1;
        } else if (op_generic(p->op) == OP_INDIR && !listed &&
                   (op_generic(p->kid[0]->op) == OP_ADDRL ||
                    op_generic(p->kid[0]->op) == OP_ADDRF) &&
                   p->kid[0]->sym[0]->sclass == LEX_REGISTER) {
            p = dag_newnode(p->op, dag_newnode(p->kid[0]->op, NULL, NULL, p->kid[0]->sym[0]), NULL,
                            NULL);
            p->count = 1;
        } else if (p->op == OP_INDIRB) {
            p->count--;
            p = dag_newnode(p->op, p->kid[0], NULL, NULL);
            p->count = 1;
            VISITKID();
        } else {
            VISITKID();
            p->sym[2] = sym_new(SYM_KTEMP, LEX_REGISTER, op_stot(p->op), SYM_SLOCAL);
            p->sym[2]->u.t.cse = p;
            sym_ref(p->sym[2], 1);
            ir_cur->symlocal(p->sym[2]);
            p->sym[2]->f.defined = 1;
            *tail = asgnnode(p->sym[2], p);
            tail = &((*tail)->link);
            if (!listed)
                p = tmpnode(p);
        }
    }

    return p;
}


/*
 *  converts dags to trees by replacing multiple refs with temporaries
 */
static dag_node_t *undag(dag_node_t *forest)
{
    dag_node_t *p;

    assert(ir_cur);

    tail = &forest;
    for (p = forest; p; p = p->link)
        if (op_generic(p->op) == OP_INDIR || (op_generic(p->op) == OP_CALL && p->count >= 1)) {
            assert(p->count >= 1);
            visit(p, 1);
        } else {
            assert(p->count == 0);
            visit(p, 1);
            *tail = p;
            tail = &p->link;
        }
    *tail = NULL;

    return forest;
}


/*
 *  generates output code by anotating forest nodes;
 *  TODO: are backing up and setting lex_cpos necessary?
 */
void (dag_gencode)(void *caller[], void *callee[])    /* sym_t */
{
    stmt_t *cp;
    lex_pos_t *ppos;

    assert(caller);
    assert(callee);
    assert(ir_cur);

    ppos = lex_cpos;
    {
        int i;
        sym_t *p, *q;
        cp = stmt_head.next->next;
        stmt_list = stmt_head.next;
        for (i = 0; (p = callee[i]) != NULL && (q = caller[i]) != NULL; i++)
            if (p->sclass != q->sclass || p->type->t.type != q->type->t.type) {
                err_entersite(&p->pos);
                dag_walk(tree_asgnid_s(p, tree_id_s(q)), 0, 0);
                err_exitsite();
            }
        stmt_list->next = cp;
        cp->prev = stmt_list;
    }
    for (cp = stmt_head.next; err_count() == 0 && cp; cp = cp->next)
        switch(cp->kind) {
            case STMT_ADDRESS:
                ir_cur->symaddr(cp->u.addr.sym, cp->u.addr.base, cp->u.addr.offset);
                break;
            case STMT_BLOCKBEG:
                {
                    void **p;    /* sym_t */
                    ir_cur->blockbeg(&cp->u.block.x);
                    for (p = cp->u.block.local; *p; p++)
                        if (S(*p)->ref != 0.0 || main_opt()->glevel)
                            ir_cur->symlocal(S(*p));
                }
                break;
            case STMT_BLOCKEND:
                ir_cur->blockend(&cp->u.begin->u.block.x);
                break;
            case STMT_DEFPOINT:
                lex_cpos = &cp->u.point.pos;
                break;
            case STMT_GEN:
            case STMT_JUMP:
            case STMT_LABEL:
                if (!ir_cur->f.want_dag)
                    cp->u.forest = undag(cp->u.forest);
                fixup(cp->u.forest);
                cp->u.forest = ir_cur->gen(cp->u.forest);
                break;
            case STMT_LOCAL:
                ir_cur->symlocal(cp->u.var);
                break;
            case STMT_SWITCH:
                break;
            default:
                assert(!"invalid code list -- should never reach here");
                break;
        }
    lex_cpos = ppos;
}


/*
 *  emits output code;
 *  TODO: are backing up and setting lex_cpos necessary?
 */
void (dag_emitcode)(void)
{
    stmt_t *cp;
    lex_pos_t *ppos;

    assert(ir_cur);

    ppos = lex_cpos;
    for (cp = stmt_head.next; err_count() == 0 && cp; cp = cp->next)
        switch(cp->kind) {
            case STMT_ADDRESS:
                break;
            case STMT_BLOCKBEG:
                break;
            case STMT_BLOCKEND:
                break;
            case STMT_DEFPOINT:
                lex_cpos = &cp->u.point.pos;
                break;
            case STMT_GEN:
            case STMT_JUMP:
            case STMT_LABEL:
                if (cp->u.forest)
                    ir_cur->emit(cp->u.forest);
                break;
            case STMT_LOCAL:
                break;
            case STMT_SWITCH:
                {
                    int i;
                    sym_t *deflab = equated(cp->u.swtch.deflab);

                    decl_defglobal(cp->u.swtch.table, INIT_SEGLIT);
                    ir_cur->initaddr(equated(cp->u.swtch.label[0]));
                    for (i = 1; i < cp->u.swtch.size; i++) {
                        long k = cp->u.swtch.value[i-1];
                        while (++k < cp->u.swtch.value[i])
                            ir_cur->initaddr(deflab);
                        ir_cur->initaddr(equated(cp->u.swtch.label[i]));
                    }
                    init_swtoseg(INIT_SEGCODE);
                }
                break;
            default:
                assert(!"invalid code list -- should never reach here");
                break;
        }
    lex_cpos = ppos;
}


/*
 *  shallow-copies a dag
 */
dag_node_t *(dag_copy)(const dag_node_t *p)
{
    dag_node_t *q;

    if (!p)
        return NULL;

    q = ARENA_CALLOC(strg_func, 1, sizeof(*q));
    memcpy(q, p, sizeof(*q));
    q->x.vr = NULL;    /* new slot to fill later */
    assert(q->count == 1 && !q->link);
    assert(!q->x.prev && !q->x.next && !q->x.prevuse);
#ifndef NDEBUG
    {
        int i;
        for (i = 0; i < NELEM(q->x.kid); i++)
            assert(!q->x.kid[i]);
    }
#endif    /* !NDEBUG */

    q->kid[0] = dag_copy(p->kid[0]);
    q->kid[1] = dag_copy(p->kid[1]);

    return q;
}


#ifndef NDEBUG
/*
 *  prints a node for debugging
 */
static void printnode(const dag_node_t *p, FILE *fp, int lev)
{
    int i, id;

    assert(p);
    assert(fp);

    id = tree_pnodeid(p);
    fprintf(fp, "[%p] %c%-4d", (void *)p, (lev == 0)? '\'': '#', id);
    for (i = 0; i < lev; i++)
        putc(' ', fp);
    fprintf(fp, "%s count=%d", op_name(p->op), p->count);
    for (i = 0; i < NELEM(p->kid) && p->kid[i]; i++)
        fprintf(fp, " #%d", tree_pnodeid(p->kid[i]));
    for (i = 0; i < NELEM(p->sym) && p->sym[i]; i++)
        fprintf(fp, " %s", p->sym[i]->name);
    putc('\n', fp);
}


/*
 *  recursively prints a dag for debugging
 */
static void printdag(const dag_node_t *p, FILE *fp, int lev)
{
    int id, i;

    assert(fp);

    if (!p || *tree_printed(id=tree_pnodeid(p)))
        return;

    *(tree_printed(id)) = 1;
    printnode(p, fp, lev);
    for (i = 0; i < NELEM(p->kid); i++)
        printdag(p->kid[i], fp, lev+1);
}


/*
 *  print a dag for debugging
 */
void (dag_print)(const dag_node_t *p, FILE *fp, int single)
{
    assert(fp);

    tree_printnew();
    if (!p) {
        if ((p = forest) != NULL)
            do {
                p = p->link;
                printdag(p, fp, 0);
            } while(p != forest);
    } else {
        int id;
        for (; p; p = p->link) {
            if (*tree_printed(id=tree_pnodeid(p)))
                fprintf(fp, "node'%d printed above\n", id);
            else
                printdag(p, fp, 0);
            if (single)
                break;
        }
    }
}
#endif    /* !NDEBUG */

/* end of dag.c */
