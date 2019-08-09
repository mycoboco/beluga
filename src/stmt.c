/*
 *  statement parsing
 */

#include <limits.h>        /* INT_MAX */
#include <stddef.h>        /* NULL */
#include <cbl/arena.h>     /* ARENA_ALLOC, ARENA_FREE */
#include <cbl/assert.h>    /* assert */
#ifndef NDEBUG
#include <stdio.h>         /* FILE, fprintf, fputs */
#endif    /* !NDEBUG */

#include "clx.h"
#include "common.h"
#include "dag.h"
#include "decl.h"
#include "enode.h"
#include "err.h"
#include "expr.h"
#include "ir.h"
#include "lex.h"
#include "main.h"
#include "op.h"
#include "simp.h"
#include "sset.h"
#include "strg.h"
#include "sym.h"
#include "tree.h"
#include "ty.h"
#include "stmt.h"

#define SWSIZE 512    /* alloc unit for case array */

/* computes density of branch table;
   ASSUMPTION: signed overflow wraps around on the host */
#define DENSITY(i, j) (((j)-(i)+1.0)/xcsf(xas(xss(v[j], v[i]), xI)))

/* checks if cp denotes unconditional jump */
#define UNCONDJMP(cp) ((cp)->kind == STMT_JUMP || (cp)->kind == STMT_SWITCH)

/* checks if integer tree p is non-zero constant;
   ASSUMPTION: no signed zero on the target */
#define ALWAYSTRUE(p) (op_generic((p)->op) == OP_CNST && xne((p)->u.v.u, xO))


/* switch handle; not stored in statement list */
struct stmt_swtch_t {
    sym_t *sym;           /* symbol to contain switch result */
    int lab;              /* label value for default label */
    sym_t *deflab;        /* symbol for default label */
    int ncase;            /* used size of case array */
    int size;             /* allocated size of case array */
    sx_t *value;          /* array of case value-label pairs */
    sym_t **label;
    const lmap_t *pos;    /* locus of switch statement */
};

/* to diagnose ambiguous else */
struct aelse {
    int amb;              /* flag to track elses */
    const lmap_t *pos;    /* locus of ambiguous else */
};


stmt_t stmt_head = { STMT_START };    /* head for statement list */
stmt_t *stmt_list = &stmt_head;       /* statement list */
double stmt_density = 0.5;            /* density for branch table */
sym_tab_t *stmt_lab;                  /* symbol table for source-code label */


/*
 *  parses an expression and make it a conditional;
 *  pointer() applies to the conditional in enode_chkcond()
 */
static tree_t *conditional(int tok, int lev, const lmap_t *posm)
{
    tree_t *p = expr_expr(tok, lev, 1, posm);

    if (!p)
        return NULL;

    if (op_generic(p->orgn->op) == OP_ASGN && !p->orgn->f.paren)
        err_dtpos(p->pos, p->kid[0], p->kid[1], ERR_EXPR_ASGNTRUTH);
    if ((p = enode_chkcond(0, p, NULL)) != NULL)
        p = enode_cond(p);

    return p;
}


/*
 *  checks if a label is equivalent to another
 */
static int equal(const sym_t *lprime, const sym_t *dst)
{
    assert(lprime);
    assert(dst);

    for (; dst; dst = dst->u.l.equatedto)
        if (lprime == dst)
            return 1;

    return 0;
}


/*
 *  generates a jump
 */
static void branch(int lab, const lmap_t *pos)
{
    stmt_t *cp;
    sym_t *p;

    assert(lab > 0);
    assert(pos);

    dag_walk(NULL, 0, 0);
    stmt_new(STMT_JUMP)->u.forest = stmt_jump(lab);
    p = stmt_list->u.forest->kid[0]->sym[0];
    assert(p == sym_findlabel(lab));
    for (cp = stmt_list->prev; cp->kind < STMT_LABEL; cp = cp->prev)
        continue;
    while (cp->kind == STMT_LABEL && cp->u.forest->op == OP_LABELV &&
           !equal(cp->u.forest->sym[0], p)) {
        stmt_eqlabel(cp->u.forest->sym[0], p);
        cp->prev->next = cp->next;
        cp->next->prev = cp->prev;
        cp = cp->prev;
        while (cp->kind < STMT_LABEL)
            cp = cp->prev;
    }
    if (UNCONDJMP(cp)) {
        sym_ref(p, -1);
        stmt_list->prev->next = NULL;
        stmt_list = stmt_list->prev;
    } else if (cp->kind == STMT_LABEL && cp->u.forest->op == OP_LABELV &&
               equal(cp->u.forest->sym[0], p))
        err_dpos(pos, ERR_STMT_INFLOOP);
}


/*
 *  parses an if statement
 */
static void ifstmt(int lab, int loop, stmt_swtch_t *swp, int lev, struct aelse *paelse)
{
    struct aelse aelse = { 0, };
    const lmap_t *posm, *ifpos = lmap_mstrip(clx_cpos);

    assert(ifpos->type == LMAP_NORMAL);

    clx_tc = clx_next();
    posm = sset_expect('(', NULL);
    stmt_defpoint(NULL);
    dag_walk(conditional(')', 0, posm), 0, lab);
    expr_refinc /= 2.0;
    if (clx_tc == ';') {
        const lmap_t *cpos = lmap_mstrip(clx_cpos),
                     *ppos = lmap_mstrip(clx_ppos);
        assert(cpos->type == LMAP_NORMAL && ppos->type == LMAP_NORMAL);
        if (cpos->u.n.py == ppos->u.n.py)
            err_dpos(clx_cpos, ERR_STMT_EMPTYBODY, "an", "if");
    }
    stmt_stmt(loop, swp, lev, NULL, &aelse, 2);
    if (clx_tc == LEX_ELSE) {
        const lmap_t *pose = clx_cpos;
        sz_t wx = lmap_mstrip(pose)->u.n.wx;
        branch(lab + 1, clx_cpos);
        clx_tc = clx_next();
        if (paelse) {
            paelse->amb = (-paelse->amb != wx && ifpos->u.n.wx != wx);
            if (paelse->amb)
                paelse->pos = pose;    /* ambiguous else */
            else if (clx_tc == LEX_IF && wx <= INT_MAX)
                paelse->amb = -(int)wx;    /* previous else */
        }
        stmt_deflabel(lab);
        stmt_stmt(loop, swp, lev, NULL, (paelse && paelse->amb <= 0)? paelse: NULL, 2);
        if (sym_findlabel(lab + 1)->ref > 0)
            stmt_deflabel(lab + 1);
    } else {
        if (aelse.amb > 0)
            (void)(err_dpos(aelse.pos, ERR_STMT_AMBELSE) &&
                   err_dpos(ifpos, ERR_STMT_AMBELSEIF));
        stmt_deflabel(lab);
    }
}


/*
 *  parses a while statement
 */
static void whilestmt(int lab, stmt_swtch_t *swp, int lev, struct aelse *paelse)
{
    tree_t *e;
    const lmap_t *pos = clx_cpos,    /* while */
                 *posm;

    expr_refinc *= 10.0;
    clx_tc = clx_next();
    posm = sset_expect('(', NULL);
    e = tree_texpr(conditional, ')', strg_func, posm);
    if (e) {
        assert(TY_ISINT(e->type));
        pos = lmap_range(pos, clx_ppos);
        branch(lab + 1, pos);
    }
    if (clx_tc == ';')
        err_dpos(clx_cpos, ERR_STMT_EMPTYBODY, "a", "while");
    stmt_deflabel(lab);
    stmt_stmt(lab, swp, lev, NULL, paelse, 2);
    stmt_deflabel(lab + 1);
    if (e) {
        stmt_defpoint(TREE_TW(e));
        if (ALWAYSTRUE(e))
            branch(lab, pos);
        else
            dag_walk(e, lab, 0);
        if (sym_findlabel(lab + 2)->ref > 0)
            stmt_deflabel(lab + 2);
    }
}


/*
 *  parses a do-while statement
 */
static void dostmt(int lab, stmt_swtch_t *swp, int lev, struct aelse *paelse)
{
    tree_t *e;
    const lmap_t *pos, *posm;

    expr_refinc *= 10.0;
    clx_tc = clx_next();
    if (clx_tc == ';')
        err_dpos(clx_cpos, ERR_STMT_EMPTYBODY, "a", "do");
    stmt_deflabel(lab);
    stmt_stmt(lab, swp, lev, NULL, paelse, 2);
    if (sym_findlabel(lab + 1)->ref > 0)
        stmt_deflabel(lab + 1);
    pos = sset_expect(LEX_WHILE, NULL);
    posm = sset_expect('(', NULL);
    stmt_defpoint(NULL);
    e = conditional(')', 0, posm);
    if (e) {
        assert(TY_ISINT(e->type));
        if (ALWAYSTRUE(e))
            branch(lab, (pos)? lmap_range(pos, clx_ppos): TREE_TW(e));
        else
            dag_walk(e, lab, 0);
        if (sym_findlabel(lab + 2)->ref > 0)
            stmt_deflabel(lab + 2);
    }
}


/*
 *  checks if an initial test in a for statement is necessary
 */
static int foldcond(tree_t *e1, tree_t *e2)
{
    int op;
    sym_t *v;

    assert(e2);

    if (!e1)
        return 0;
    op = op_generic(e2->op);

    if (op_generic(e1->op) == OP_ASGN && OP_ISADDR(e1->kid[0]->op) &&
        op_generic(e1->kid[1]->op) == OP_CNST) {
        v = e1->kid[0]->u.sym;
        e1 = e1->kid[1];
    } else
        return 0;
    if ((op == OP_LE || op == OP_LT || op == OP_EQ || op == OP_NE || op == OP_GT || op == OP_GE) &&
        op_generic(e2->kid[0]->op) == OP_INDIR && OP_ISADDR(e2->kid[0]->kid[0]->op) &&
        e2->kid[0]->kid[0]->u.sym == v && e2->kid[1]->op == e1->op) {
        e1 = simp_tree(op, e2->type, e1, e2->kid[1], e2->pos);    /* type not from op suffix;
                                                                     intended to fold integers */
        if (op_optype(e1->op) == OP_CNST+OP_I)
            return xt(e1->u.v.s);
    }

    return 0;
}


/*
 *  parses a for statement
 */
static void forstmt(int lab, stmt_swtch_t *swp, int lev, struct aelse *paelse)
{
    int once, inf;
    tree_t *e1, *e2, *e3;
    const lmap_t *pos = clx_cpos,    /* for */
                 *pos2,              /* 2nd expression */
                 *pos3,              /* 3rd expression */
                 *posm;

    once = inf = 0;
    e1 = e2 = e3 = NULL;

    clx_tc = clx_next();
    posm = sset_expect('(', NULL);
    stmt_defpoint(NULL);
    if (clx_isexpr()) {
        e1 = tree_texpr(expr_expr0, ';', strg_func, NULL);
        dag_walk(e1, 0, 0);
    } else
        sset_expect(';', NULL);
    pos2 = clx_cpos;
    expr_refinc *= 10.0;
    if (clx_isexpr()) {
        if ((e2 = tree_texpr(conditional, ';', strg_func, NULL)) != NULL)
            pos2 = TREE_TW(e2);
    } else {
        if (clx_tc == ';')
            inf = 1;
        sset_expect(';', NULL);
    }
    assert(!e2 || TY_ISINT(e2->type));
    pos3 = clx_cpos;
    if (clx_isexpr()) {
        e3 = tree_texpr(expr_expr0, ')', strg_func, posm);
        if (e3)
            pos3 = TREE_TW(e3);
    } else
        sset_test(')', sset_expr, posm);
    pos = lmap_range(pos, clx_ppos);
    if (e2) {
        once = foldcond(e1, e2);
        if (!once)
            branch(lab + 3, pos);
    }
    if (clx_tc == ';')
        err_dpos(clx_cpos, ERR_STMT_EMPTYBODY, "a", "for");
    stmt_deflabel(lab);
    stmt_stmt(lab, swp, lev, NULL, paelse, 2);
    stmt_deflabel(lab + 1);
    stmt_defpoint(pos3);
    if (e3)
        dag_walk(e3, 0, 0);
    if (e2 && !ALWAYSTRUE(e2)) {
        if (!once)
            stmt_deflabel(lab + 3);
        stmt_defpoint(pos2);
        dag_walk(e2, lab, 0);
    } else if (e2 || inf) {
        stmt_defpoint(pos2);
        branch(lab, pos);
    }
    if (sym_findlabel(lab + 2)->ref > 0)
        stmt_deflabel(lab + 2);
}


/*
 *  generates a statement list that compares two integers
 */
static void cmp(int op, sym_t *p, sx_t n, int lab, tree_pos_t *tpos)
{
    ty_t *ty;

    assert(p);
    assert(lab > 0);
    assert(ty_longtype);    /* ensures types initialized */

    ty = ty_scounter(TY_UNQUAL(p->type));
    dag_listnode(tree_cmp(op, tree_id(p, tpos), tree_sconst(n, ty, tpos), ty, tpos), lab, 0);
}


/*
 *  generates selection code for a switch;
 *  ASSUMPTION: signed/unsigned integers are compatible on the host;
 *  ASSUMPTION: code location can be represented by void pointer on the target
 */
static void swcode(stmt_swtch_t *swp, int b[], int lb, int ub)
{
    sx_t *v;
    int hilab, lolab;
    int l, u, k = (lb + ub)/2;
    const lmap_t *pos;
    tree_pos_t *tpos;

    assert(swp);
    assert(b);
    assert(lb >= 0 && ub >= lb);
    assert(ty_voidptype);    /* ensures types initialized */
    assert(ir_cur);

    pos = swp->pos;
    tpos = tree_npos1(pos);

    v = swp->value;    /* used in DENSITY() */
    hilab = lolab = swp->deflab->u.l.label;
    if (k > lb && k < ub) {
        lolab = sym_genlab(1);
        hilab = sym_genlab(1);
    } else if (k > lb)
        lolab = sym_genlab(1);
    else if (k < ub)
        hilab = sym_genlab(1);
    else
        assert(lolab == hilab && lolab == swp->deflab->u.l.label);

    l = b[k];
    u = b[k+1] - 1;
    if (u - l + 1 <= 3) {
        int i;
        for (i = l; i <= u; i++)
           cmp(OP_EQ, swp->sym, v[i], swp->label[i]->u.l.label, tpos);
        if (k > lb && k < ub)
           cmp(OP_GT, swp->sym, v[u], hilab, tpos);
        else if (k > lb)
           cmp(OP_GT, swp->sym, v[u], hilab, tpos);
        else if (k < ub)
           cmp(OP_LT, swp->sym, v[l], lolab, tpos);
        else
           branch(lolab, pos);
    } else {
        sym_t *table;

        table = sym_new(SYM_KGEN, LEX_STATIC, ty_array(ty_voidptype, u-l+1, NULL), SYM_SLABEL);
        ir_cur->symgsc(table);
        cmp(OP_LT, swp->sym, v[l], lolab, tpos);
        cmp(OP_GT, swp->sym, v[u], hilab, tpos);
        dag_walk(tree_new(OP_JMP, ty_voidtype,
                     tree_indir(
                         tree_add(OP_ADD,
                             enode_pointer(tree_id(table, tpos)),
                             tree_sub(OP_SUB,
                                 enode_cast(tree_id(swp->sym, tpos), ty_longtype, 0, NULL),
                                 tree_sconst(v[l], ty_longtype, tpos), NULL, tpos),
                             NULL, tpos),
                         ty_voidptype, 0, tpos),
                     NULL, tpos),
                 0, 0);
        stmt_new(STMT_SWITCH);
        stmt_list->u.swtch.table = table;
        stmt_list->u.swtch.sym = swp->sym;
        stmt_list->u.swtch.deflab = swp->deflab;
        stmt_list->u.swtch.size = u - l + 1;
        stmt_list->u.swtch.value = &v[l];
        stmt_list->u.swtch.label = &swp->label[l];
        if (xges(xas(xss(v[u], v[l]), xI), xis(10000)))
            err_dpos(pos, ERR_STMT_HUGETABLE);
    }
    if (k > lb) {
        stmt_deflabel(lolab);
        swcode(swp, b, lb, k-1);
    }
    if (k < ub) {
        stmt_deflabel(hilab);
        swcode(swp, b, k+1, ub);
    }
}


/*
 *  generates branch tables for a switch
 */
static void swgen(stmt_swtch_t *swp)
{
    sx_t *v;
    int *bucket, k, n;

    assert(swp);

    v = swp->value;    /* used in DENSITY() */
    bucket = ARENA_ALLOC(strg_func, (swp->ncase+1)*sizeof(*bucket));
    for (n = k = 0; k < swp->ncase; k++, n++) {
        bucket[n] = k;
        while (n > 0 && DENSITY(bucket[n-1], k) >= stmt_density)
            n--;
    }
    bucket[n] = swp->ncase;
    swcode(swp, bucket, 0, n-1);
}


/*
 *  parses a switch statement
 */
static void swstmt(int lab, int loop, int lev, struct aelse *paelse)
{
    tree_t *e;
    stmt_swtch_t sw;
    stmt_t *head, *tail;
    const lmap_t *posm;

    sw.pos = clx_cpos;
    clx_tc = clx_next();
    posm = sset_expect('(', NULL);
    stmt_defpoint(NULL);
    e = expr_expr(')', 0, 1, posm);
    sw.pos = lmap_range(sw.pos, clx_ppos);
    if (e) {
        if (TY_ISINTEGER(e->type)) {
            e = enode_cast(e, ty_ipromote(e->type), 0, NULL);
            if (op_generic(e->op) == OP_INDIR && OP_ISADDR(e->kid[0]->op) &&
                TY_UNQUAL(e->kid[0]->u.sym->type)->t.type == e->type->t.type &&
                !TY_ISVOLATILE(e->kid[0]->u.sym->type)) {
                sw.sym = e->kid[0]->u.sym;
                dag_walk(NULL, 0, 0);
            } else {
                sw.sym = sym_new(SYM_KGEN, LEX_REGISTER, e->type->t.type, sym_scope);
                stmt_local(sw.sym);
                dag_walk(tree_asgnid(sw.sym, e, e->pos), 0, 0);
            }
        } else {
            err_dpos(TREE_TW(e), ERR_STMT_SWTCHNOINT);
            sw.sym = NULL;
        }
    } else
        sw.sym = NULL;

    head = stmt_new(STMT_SWITCH);
    sw.lab = lab;
    sw.deflab = NULL;
    sw.ncase = 0;
    sw.size = SWSIZE;
    sw.value = ARENA_ALLOC(strg_func, SWSIZE * sizeof(*sw.value));
    sw.label = ARENA_ALLOC(strg_func, SWSIZE * sizeof(*sw.label));
    expr_refinc /= 10.0;
    stmt_stmt(loop, &sw, lev, NULL, paelse, 2);
    if (!sw.deflab) {
        sw.deflab = sym_findlabel(lab);
        stmt_deflabel(lab);
        if (sw.ncase == 0 && sw.sym)
            err_dpos(sw.pos, ERR_STMT_SWTCHNOCASE);
    }
    if (sym_findlabel(lab + 1)->ref > 0)
        stmt_deflabel(lab + 1);
    tail = stmt_list;
    stmt_list = head->prev;
    stmt_list->next = head->prev = NULL;
    if (sw.sym && sw.ncase > 0)
        swgen(&sw);
    else
        branch(lab, sw.pos);
    head->next->prev = stmt_list;
    stmt_list->next = head->next;
    stmt_list = tail;
}


/*
 *  adds a case label into a table;
 *  ASSUMPTION: signed/unsigned integers are compatible on the host
 */
static void caselabel(stmt_swtch_t *swp, sx_t val, int lab, const lmap_t *pos)
{
    int k;

    assert(swp);
    assert(pos);

    if (swp->ncase >= swp->size) {
        sx_t *valarr = swp->value;
        sym_t **labarr = swp->label;
        swp->size *= 2;
        swp->value = ARENA_ALLOC(strg_func, swp->size*sizeof(*swp->value));
        swp->label = ARENA_ALLOC(strg_func, swp->size*sizeof(*swp->label));
        for (k = 0; k < swp->ncase; k++) {
            swp->value[k] = valarr[k];
            swp->label[k] = labarr[k];
        }
    }

    for (k = swp->ncase; k > 0 && xges(swp->value[k-1], val); k--) {
        swp->value[k] = swp->value[k-1];
        swp->label[k] = swp->label[k-1];
    }
    pos = lmap_range(pos, clx_ppos);
    if (k < swp->ncase && xe(swp->value[k], val)) {
        int d;
        ty_t *ty = ty_ipromote(swp->sym->type);
        if (TY_ISUNSIGN(ty))
            d = err_dpos(pos, ERR_STMT_DUPCASEU, *(ux_t *)&val);
        else
            d = err_dpos(pos, ERR_STMT_DUPCASES, val);
        if (d && xe(swp->label[k]->u.l.val, val))
            err_dpos(swp->label[k]->pos, ERR_PARSE_PREVDEF);
    }
    swp->value[k] = val;
    swp->label[k] = sym_findlabel(lab);
    swp->label[k]->pos = pos;
    swp->label[k]->u.l.val = val;
    if (swp->ncase++ == TL_NCASE_STD)
        (void)(err_dpos(pos, ERR_STMT_MANYCASE) &&
               err_dpos(pos, ERR_STMT_MANYCASESTD, (long)TL_NCASE_STD));
}


/*
 *  installs a source-code label
 */
static void stmtlabel(void) {
    sym_t *p = sym_lookup(clx_tok, stmt_lab);

    if (!p)
        p = sym_new(SYM_KLABEL, clx_tok, clx_cpos, &stmt_lab);
    if (p->f.defined)
        (void)(err_dpos(clx_cpos, ERR_STMT_DUPLABEL, p, "") &&
               err_dpos(p->pos, ERR_PARSE_PREVDEF));
    else
        stmt_deflabel(p->u.l.label);
    p->f.defined = 1;
    clx_tc = clx_next();
    sset_expect(':', NULL);
}


/*
 *  returns a symbol if p is based on the address of a local or parameter
 */
static sym_t *localaddr(const tree_t *p)
{
    assert(ir_cur);

    if (!p)
        return NULL;

    switch(op_generic(p->op)) {
        case OP_INDIR:
            if (!ir_cur->f.want_argb && op_generic(p->kid[0]->op) == OP_ADDRF &&
                TY_ISSTRUNI(p->kid[0]->u.sym->type))
                return p->kid[0]->u.sym;
        case OP_CALL:
            return NULL;
        case OP_ARG:
            assert(!"invalid operation code -- should never reach here");
            return NULL;
        case OP_ADDRL:
        case OP_ADDRF:
            return p->u.sym;
        case OP_RIGHT:
        case OP_ASGN:
            if (p->kid[1])
                return localaddr(p->kid[1]);
            return localaddr(p->kid[0]);
        case OP_COND:
            {
                sym_t *q;
                assert(p->kid[1] && p->kid[1]->op == OP_RIGHT);
                if ((q = localaddr(p->kid[1]->kid[0])) != NULL)
                    return q;
                return localaddr(p->kid[1]->kid[1]);
            }
        default:
            {
                sym_t *q;
                if (p->kid[0] && (q = localaddr(p->kid[0])) != NULL)
                    return q;
                return localaddr(p->kid[1]);
            }
    }
}


/*
 *  generates code for a return;
 *  ASSUMPTION: signed types can be used instead of unsigned counterparts;
 *  ASSUMPTION: pointers can be returned as an integer
 */
void (stmt_retcode)(tree_t *p, const lmap_t *pos)
{
    ty_t *ty;
    tree_pos_t *tpos;

    assert(pos);
    assert(ty_voidtype);    /* ensures types initialized */

    if (!p)
        return;
    p = enode_value(enode_pointer(p));
    ty = enode_tcasgnty(ty_freturn(decl_cfunc->type), p, pos, NULL);
    if (!ty || ty->t.type == ty_voidtype) {
        if (!ty)
            err_dpos(TREE_TW(p), ERR_STMT_ILLRETTYPE, p->type, ty_freturn(decl_cfunc->type));
        return;
    }
    p = enode_cast(p, ty, 1, pos);
    if (decl_retv) {
        assert(TY_ISPTR(decl_retv->type));
        assert(!TY_ISQUAL(decl_retv->type));
        tpos = p->orgn->pos;
        p = (tree_iscallb(p))?
                tree_right(
                    tree_new(OP_CALL+OP_B, p->type,
                             p->kid[0]->kid[0], tree_id(decl_retv, tpos), tpos),
                    tree_indir(tree_id(decl_retv, tpos), decl_retv->type->type, 0, tpos),
                    p->type, tpos):
                tree_asgnf(OP_ASGN,
                    tree_indir(tree_id(decl_retv, tpos), TY_UNQUAL(ty), 0, tpos), p, NULL, tpos);
        dag_walk(p, 0, 0);
        return;
    }
    p = enode_cast(p, ty_ipromote(p->type), 0, pos);
    if (TY_ISPTR(p->type)) {
        sym_t *q = localaddr(p);
        if (q)
            err_dmpos(TREE_TW(p), ERR_STMT_RETLOCAL, pos, NULL,
                      (q->scope == SYM_SPARAM)? "parameter": "local", q, "");
        p = enode_cast(p, ty_ptruinttype, 0, NULL);
    }
    dag_walk(tree_new(OP_RET+OP_SFXW(p->type), p->type, p, NULL, tree_npos1(pos)), 0, 0);
}


/*
 *  generates and appends a new statement entry
 */
stmt_t *(stmt_new)(int kind)
{
    stmt_t *cp;

    cp = ARENA_ALLOC(strg_func, sizeof(*cp));
    cp->kind = kind;
    cp->prev = stmt_list;
    cp->next = NULL;
    stmt_list->next = cp;
    stmt_list = cp;

    return cp;
}


/*
 *  appends a new LOCAL statement entry
 */
void (stmt_local)(sym_t *p)
{
    assert(p);

    if (!p->f.defined) {
        stmt_new(STMT_LOCAL)->u.var = p;
        p->f.defined = 1;
    }
}


/*
 *  appends a new DEFPOINT statement entry
 */
void (stmt_defpoint)(const lmap_t *pos) {
    stmt_t *cp = stmt_new(STMT_DEFPOINT);

    cp->u.point.pos = (pos)? pos: clx_cpos;
}


/*
 *  defines a label
 */
void (stmt_deflabel)(int lab)
{
    stmt_t *cp;
    sym_t *p = sym_findlabel(lab);

    assert(lab > 0);

    dag_walk(NULL, 0, 0);
    stmt_new(STMT_LABEL)->u.forest = dag_newnode(OP_LABELV, NULL, NULL, p);
    for (cp = stmt_list->prev; cp->kind <= STMT_LABEL; cp = cp->prev)
        continue;
    while (cp->kind == STMT_JUMP && cp->u.forest->kid[0] &&
           op_generic(cp->u.forest->kid[0]->op) == OP_ADDRG && cp->u.forest->kid[0]->sym[0] == p) {
        sym_ref(p, -1);
        cp->prev->next = cp->next;
        cp->next->prev = cp->prev;
        cp = cp->prev;
        while (cp->kind <= STMT_LABEL)
            cp = cp->prev;
    }
}


/*
 *  marks a label as equivalent to another
 */
void (stmt_eqlabel)(sym_t *old, sym_t *new)
{
    assert(old);
    assert(!old->u.l.equatedto);
    assert(new);

    old->u.l.equatedto = new;
    sym_ref(new, 1);
}


/*
 *  composes a Jump node
 */
dag_node_t *(stmt_jump)(int lab)
{
    sym_t *p;

    assert(lab > 0);

    p = sym_findlabel(lab);
    sym_ref(p, 1);
    return dag_newnode(OP_JMPV, dag_newnode(op_addr(G), NULL, NULL, p), NULL, NULL);
}


/*
 *  checks if unreachable code to be added
 */
void (stmt_chkreach)(void)
{
    int chk;
    stmt_t *cp;

    assert(stmt_list);

    switch (clx_tc) {
        case LEX_IF:
        case LEX_WHILE:
        case LEX_DO:
        case LEX_FOR:
        case LEX_BREAK:
        case LEX_CONTINUE:
        case LEX_SWITCH:
        case LEX_RETURN:
        case ';':
        case LEX_GOTO:
            chk = 1;
            break;
        case '{':
            chk = 2;    /* SWITCH excluded */
            break;
        case LEX_ID:
            chk = (clx_peek() != ':');
            break;
        default:
            chk = (clx_isexpr() || clx_issdecl(CLX_TYLA));
            break;
    }
    if (!chk)
        return;

    for (cp = stmt_list; cp->kind < STMT_LABEL; cp = cp->prev)
        continue;
    if ((chk == 1 && UNCONDJMP(cp)) || (chk == 2 && cp->kind == STMT_JUMP))
        err_dpos(lmap_pin(clx_cpos), ERR_STMT_UNREACHABLE);
}


/*
 *  parses a statement;
 *  ASSUMPTION: signed/unsigned integers are compatible on the host
 */
void (stmt_stmt)(int loop, stmt_swtch_t *swp, int lev, const lmap_t *post,    /* label or { */
                 void *paelse, int diag)    /* struct aelse */
{
    double ref = expr_refinc;
    const lmap_t *pos = clx_cpos;    /* current statement */

    assert(ty_voidtype);    /* ensures types initialized */

    stmt_chkreach();
    if (lev == TL_BLOCK_STD+1)
        (void)(err_dpos(lmap_pin((post)? post: pos), ERR_STMT_MANYNEST) &&
               err_dpos(lmap_pin((post)? post: pos), ERR_STMT_MANYNESTSTD, (long)TL_BLOCK_STD));
    if (clx_tc == '}' || clx_ispdecl()) {
        switch(diag) {
            case 0:    /* legal */
                break;
            case 1:    /* after label */
                err_dpos(lmap_after(clx_ppos), ERR_STMT_LABELSTMT);
                break;
            default:    /* other cases */
                err_dpos(lmap_after(clx_ppos), ERR_STMT_STMTREQ, clx_tc);
                if (clx_ispdecl())
                    decl_errdecl(CLX_TYLA);    /* CLX_TYLA from clx_ispdecl() */
                break;
        }
        return;
    }
    switch (clx_tc) {
        case LEX_IF:
            ifstmt(sym_genlab(2), loop, swp, lev+1, paelse);
            break;
        case LEX_WHILE:
            whilestmt(sym_genlab(3), swp, lev+1, paelse);
            break;
        case LEX_DO:
            dostmt(sym_genlab(3), swp, lev+1, NULL);
            sset_expect(';', NULL);
            break;
        case LEX_FOR:
            forstmt(sym_genlab(4), swp, lev+1, paelse);
            break;
        case LEX_BREAK:
            dag_walk(NULL, 0, 0);
            stmt_defpoint(NULL);
            if (swp && swp->lab > loop)
                branch(swp->lab + 1, pos);
            else if (loop > 0)
                branch(loop + 2, pos);
            else
                err_dpos(clx_cpos, ERR_STMT_ILLBREAK);
            clx_tc = clx_next();
            sset_expect(';', NULL);
            break;
        case LEX_CONTINUE:
            dag_walk(NULL, 0, 0);
            stmt_defpoint(NULL);
            if (loop > 0)
                branch(loop + 1, pos);
            else
                err_dpos(clx_cpos, ERR_STMT_ILLCONTINUE);
            clx_tc = clx_next();
            sset_expect(';', NULL);
            break;
        case LEX_SWITCH:
            swstmt(sym_genlab(2), loop, lev+1, paelse);
            break;
        case LEX_CASE:
            {
                int lab = sym_genlab(1);
                const lmap_t *posl = clx_cpos;
                stmt_deflabel(lab);
                while (clx_tc == LEX_CASE) {
                    tree_t *p;
                    pos = clx_cpos;    /* case */
                    clx_tc = clx_next();
                    p = simp_intexpr(0, NULL, 0, xO, "case label", NULL);
                    if (p && swp && swp->sym) {
                        simp_needconst++;
                        p = enode_cast(p, swp->sym->type, 1, pos);
                        simp_needconst--;
                        caselabel(swp, p->u.v.s, lab, pos);
                    }
                    sset_expect(':', NULL);
                }
                if (!swp)
                    err_dpos(lmap_range(posl, clx_ppos), ERR_STMT_INVCASE);
                stmt_stmt(loop, swp, lev, pos, paelse, 1);
            }
            break;
        case LEX_DEFAULT:
            if (!swp)
                err_dpos(clx_cpos, ERR_STMT_INVDEFAULT);
            else if (swp->deflab)
                (void)(err_dpos(clx_cpos, ERR_STMT_DUPDEFAULT) &&
                       err_dpos(swp->deflab->pos, ERR_PARSE_PREVDEF));
            else {
                swp->deflab = sym_findlabel(swp->lab);
                swp->deflab->pos = clx_cpos;
                stmt_deflabel(swp->deflab->u.l.label);
            }
            clx_tc = clx_next();
            sset_expect(':', NULL);
            stmt_stmt(loop, swp, lev, pos, paelse, 1);
            break;
        case LEX_RETURN:
            {
                ty_t *rty = ty_freturn(decl_cfunc->type);
                ty_t *uty = TY_UNQUAL(rty)->t.type;
                clx_tc = clx_next();
                stmt_defpoint(NULL);
                if (clx_isexpr()) {
                    if (uty == ty_voidtype) {
                        tree_t *e = expr_expr(0, 0, 1, NULL);
                        err_dpos(TREE_NW(e), ERR_STMT_EXTRARETURN);
                        stmt_retcode(NULL, pos);
                    } else
                        stmt_retcode(expr_expr(0, 0, 1, NULL), pos);
                } else {
                    if (DECL_NORET(decl_cfunc->type))
                        err_dpos(lmap_after(clx_ppos),
                                 (rty->t.type != ty_inttype ||
                                  !decl_cfunc->type->u.f.implint)?    /* for warning code */
                                     ERR_STMT_NORETURN: ERR_STMT_NORETURNSTD);
                    stmt_retcode(NULL, pos);
                }
                branch(decl_cfunc->u.f.label, pos);
            }
            sset_expect(';', NULL);
            break;
        case '{':
            decl_compound(loop, swp, lev+1);
            sset_expect('}', pos);
            break;
        case ';':
            stmt_defpoint(NULL);
            clx_tc = clx_next();
            break;
        case LEX_GOTO:
            dag_walk(NULL, 0, 0);
            stmt_defpoint(NULL);
            clx_tc = clx_next();
            if (clx_tc == LEX_ID) {
                sym_t *p = sym_lookup(clx_tok, stmt_lab);
                if (!p)
                    p = sym_new(SYM_KLABEL, clx_tok, clx_cpos, &stmt_lab);
                p->f.wregister = 1;
                if (main_opt()->xref)
                    sym_use(p, clx_cpos);
                branch(p->u.l.label, lmap_range(pos, clx_cpos));
                clx_tc = clx_next();
            } else
                err_dpos(lmap_after(clx_ppos), ERR_STMT_GOTONOLAB);
            sset_expect(';', NULL);
            break;
        case LEX_ELSE:
            err_dpos(clx_cpos, ERR_STMT_INVELSE);
            clx_tc = clx_next();
            break;
        case LEX_ID:
            if (clx_peek() == ':') {
                decl_chkid(clx_tok, clx_cpos, stmt_lab, 0);
                stmtlabel();
                stmt_stmt(loop, swp, lev, pos, paelse, 1);
                break;
            }
            /* no break */
        default:
            stmt_defpoint(NULL);
            if (!clx_isexpr()) {
                err_dpos(lmap_pin(clx_cpos), ERR_STMT_ILLSTMT);
                clx_tc = clx_next();
            } else {
                tree_t *e = expr_expr0(0, 0, NULL);
                DAG_LISTNODE(e, 0, 0);
                if (dag_nodecount == 0 || dag_nodecount > 200 || main_opt()->glevel)
                    dag_walk(NULL, 0, 0);
                ARENA_FREE(strg_stmt);
            }
            sset_expect(';', NULL);
            break;
    }
    expr_refinc = ref;
}


#ifndef NDEBUG
/*
 *  prints the current statement list
 */
void (stmt_print)(FILE *fp)
{
    stmt_t *p;

    assert(fp);

    for (p = &stmt_head; p; p = p->next)
        switch(p->kind) {
            case STMT_BLOCKBEG:    /* beginning of block */
                fprintf(fp, "\n  # Block begins (lev:%d, ptr:%p)\n", p->u.block.scope, (void *)p);
                break;
            case STMT_BLOCKEND:    /* end of block */
                assert(p->u.begin);
                fprintf(fp, "  # Block ends (lev:%d, ptr:%p)\n\n", p->u.begin->u.block.scope,
                        (void *)p->u.begin);
                break;
            case STMT_LOCAL:    /* local symbol */
                assert(p->u.var && p->u.var->name);
                fprintf(fp, "  - Local (symbol:%s)\n", p->u.var->name);
                break;
            case STMT_ADDRESS:    /* address symbol */
                assert(p->u.addr.sym && p->u.addr.sym->name);
                assert(p->u.addr.base && p->u.addr.base->name);
                fprintf(fp, "  - Address (symbol:%s=%s+%"FMTSZ"d)\n", p->u.addr.sym->name,
                        p->u.addr.base->name, p->u.addr.offset);
                break;
            case STMT_DEFPOINT:    /* execution point */
                fprintf(fp, "  . Exec point (%s)\n", lmap_out(p->u.point.pos));
                break;
            case STMT_LABEL:    /* label */
                assert(p->u.forest->op == OP_LABELV);
                assert(p->u.forest->sym[0] && p->u.forest->sym[0]->name);
                fprintf(fp, "  > Label (label:%s)\n", p->u.forest->sym[0]->name);
                break;
            case STMT_START:    /* beginning of statement list */
                fputs("\n= Start\n", fp);
                break;
            case STMT_GEN:    /* expression or statement */
                fputs("  - Expression\n", fp);
                dag_print(p->u.forest, fp, 0);
                break;
            case STMT_JUMP:    /* branch */
                assert(p->u.forest->op == OP_JMPV);
                assert(op_generic(p->u.forest->kid[0]->op) == OP_ADDRG);
                assert(p->u.forest->kid[0]->sym[0] && p->u.forest->kid[0]->sym[0]->name);
                fprintf(fp, "  > Jump (label:%s)\n", p->u.forest->kid[0]->sym[0]->name);
                break;
            case STMT_SWITCH:    /* branch table */
                assert(p->u.swtch.sym && p->u.swtch.sym->name);
                fprintf(fp, "  > Switch brach (symbol:%s, size:%d)\n", p->u.swtch.sym->name,
                        p->u.swtch.size);
                {
                    int i;
                    for (i = 0; i < p->u.swtch.size; i++) {
                        assert(p->u.swtch.value + i);
                        assert(p->u.swtch.label + i && p->u.swtch.label[i]->name);
                        fprintf(fp, "    [%s:%s]\n", xtud(p->u.swtch.value[i]),
                                p->u.swtch.label[i]->name);
                    }
                }
                break;
            default:
                assert(!"invalid statement kind -- should never reach here");
                break;
        }
}
#endif    /* !NDEBUG */

/* end of stmt.c */
