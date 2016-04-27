/*
 *  experssion parsing
 */

#include <stddef.h>        /* NULL */
#include <cbl/assert.h>    /* assert */
#include <cdsl/hash.h>     /* hash_new */

#include "decl.h"
#include "enode.h"
#include "err.h"
#include "in.h"
#include "ir.h"
#include "lex.h"
#include "main.h"
#include "op.h"
#include "simp.h"
#include "stmt.h"
#include "strg.h"
#include "sym.h"
#include "tree.h"
#include "ty.h"
#include "expr.h"


double expr_refinc = 1.0;    /* weight for reference counter */


/* precedence of operators */
static char prec[] = {
#define xx(a, b, c, d, e, f, g, h) c,
#define yy(a, b, c, d, e, f, g, h) c,
#include "xtoken.h"
};


/*
 *  parses a primary expression:
 *      prim-exp:
 *          integer-constant
 *          floating-constant
 *          string-literal
 *          identifier
 */
static tree_t *expr_prim(void)
{
    tree_t *p;

    assert(ty_inttype);    /* ensures types initialized */
    assert(ir_cur);

    assert(lex_tc != '(');

    err_entersite(lex_cpos);    /* enters with token for primary expression */
    switch(lex_tc) {
        case LEX_CCON:
        case LEX_ICON:
        case LEX_FCON:
            if (lex_sym) {
                p = tree_new_s(OP_CNST+op_sfx(lex_sym->type), lex_sym->type, NULL, NULL);
                p->u.v = lex_sym->u.c.v;
            } else
                p = NULL;
            break;
        case LEX_SCON:
            lex_sym->u.c.v.hp = hash_new(lex_sym->u.c.v.hp, lex_sym->type->size);
            lex_sym = sym_findconst(lex_sym->type, lex_sym->u.c.v);
            if (!lex_sym->u.c.loc)
                lex_sym->u.c.loc = sym_new(SYM_KGEN, LEX_STATIC, lex_sym->type, SYM_SGLOBAL);
            lex_sym->u.c.loc->f.outofline = 1;
            p = tree_id_s(lex_sym->u.c.loc);
            break;
        case LEX_ID:
            if (!lex_sym) {
                sym_t *q = sym_new(SYM_KORDIN, lex_tok, lex_cpos,
                                   LEX_AUTO, ty_inttype,    /* modified later */
                                   (sym_scope < SYM_SPARAM)? strg_perm: strg_func);
                lex_tc = lex_next();
                if (lex_tc == '(') {
                    sym_t *r = sym_lookup(lex_tok, sym_extern);
                    q->sclass = LEX_EXTERN;
                    err_entersite(lex_cpos);    /* enters with ( */
                    q->type = ty_func_s(ty_inttype, NULL, 1);
                    err_exitsite();    /* exits from ( */
                    err_issuep(&q->pos, ERR_EXPR_IMPLDECL);
                    err_issuep(&q->pos, ERR_EXPR_IMPLDECLSTD);
                    err_issuep(&q->pos, ERR_PARSE_NOPROTO);
                    if (!decl_chkid(lex_tok, &q->pos, sym_global, 1))
                        decl_chkid(lex_tok, &q->pos, sym_extern, 1);
                    if (r && !ty_equiv(r->type, q->type, 1))
                        err_issuep(&q->pos, ERR_PARSE_REDECL2, q, " an identifier", &r->pos);
                    ir_cur->symgsc(q);
                    if (!r)
                        sym_new(SYM_KEXTERN, q->name, &q->pos, LEX_EXTERN, q->type);
                    p = tree_id_s(q);
                } else {
                    err_issuep(&q->pos, ERR_EXPR_NOID, q, "");
                    q->f.undecl = 1;
                    if (q->scope == SYM_SGLOBAL)
                        ir_cur->symgsc(q);
                    else
                        stmt_local(q);
                    p = NULL;
                }
                assert(q->type && q->sclass);
                if (main_opt()->xref)
                    sym_use(lex_sym, lex_cpos);
                err_exitsite();    /* exits from token for primary expression */
                return p;
            }
            if (main_opt()->xref)
                sym_use(lex_sym, lex_cpos);
            if (lex_sym->sclass == LEX_ENUM)
                p = tree_sconst_s(lex_sym->u.value, ty_inttype);
            else if (!lex_sym->f.undecl) {
                if (lex_sym->sclass == LEX_TYPEDEF) {
                    err_issuep(lex_cpos, ERR_EXPR_ILLTYPEDEF, lex_sym, "");
                    p = NULL;
                } else
                    p = tree_id_s(lex_sym);
            } else
                p = NULL;
            break;
        default:
            err_issuep(lex_epos(), ERR_EXPR_ILLEXPR);
            if (!tree_optree_s[lex_tc])    /* for better diagnostics */
                switch(lex_tc) {
                    case ')':
                    case ',':
                    case '.':
                    case LEX_DEREF:
                    case ':':
                    case ';':
                    case '?':
                    case '[':
                    case ']':
                    case '}':
                        break;
                    default:
                        lex_tc = lex_next();
                }
            err_exitsite();    /* exits from token for primary expression */
            return NULL;
    }
    lex_tc = lex_next();

    err_exitsite();    /* exits from token for primary expression */
    return p;
}


/*
 *  parses a postfix expression:
 *      postfix-exp:
 *          primary-exp { postfix-op }
 */
static tree_t *expr_postfix(tree_t *p)
{
    assert(ty_inttype);    /* ensures types initialized */

    for (;;) {
        err_entersite(lex_cpos);    /* enters with postfix operator */
        switch(lex_tc) {
            case LEX_INCR:
            case LEX_DECR:
                {
                    tree_t *r = tree_casgn_s(lex_tc, p, tree_sconst_s(1, ty_inttype));
                    p = (r)? tree_right_s(tree_right_s(p, r, NULL), p, NULL): NULL;
                    lex_tc = lex_next();
                }
                break;
            case '[':
                {
                    tree_t *q;
                    lex_tc = lex_next();
                    q = expr_expr(']', 0, 0);
                    if (q && TY_UNQUAL(q->type)->t.type == ty_chartype &&
                        op_generic(q->op) != OP_CNST)
                        err_issuep(&q->pos, ERR_EXPR_CHARSUBSCR);
                    p = tree_indir_s(tree_optree_s['+'](OP_SUBS, p, q, NULL), NULL, 1);
                }
                break;
            case '(':
                /* no call to lex_next() to let tree_pcall() do */
                p = tree_pcall(p);
                break;
            case '.':
            case LEX_DEREF:
                /* no call to lex_next() to let tree_dot_s() do */
                p = tree_dot_s(lex_tc, p);
                break;
            default:
                err_exitsite();    /* exits from postfix operator */
                return p;
        }
        err_exitsite();    /* exits from postfix operator */
    }

    /* assert(!"impossible control flow -- should never reach here");
       return p; */
}


/*
 *  parses a unary expression:
 *      unary-exp:
 *          postfix-exp
 *          unary-op unary-exp
 *          ( type-name ) unary-exp
 *          sizeof unary-exp
 *          sizeof ( type-name )
 */
static tree_t *expr_unary(int lev)
{
    int tc;
    tree_t *p;

    assert(ty_inttype);    /* ensures types initialized */

    err_entersite(lex_cpos);    /* enters with unary operator */
    switch(tc = lex_tc) {
        case '*':
            lex_tc = lex_next();
            p = tree_indir_s(expr_unary(lev), NULL, 1);
            break;
        case '&':
            lex_tc = lex_next();
            p = tree_addr_s(expr_unary(lev), NULL, 1);
            /* checked here to allow assignment to temporary object */
            if (p && OP_ISADDR(p->op)) {
                if (p->u.sym->sclass == LEX_REGISTER)
                    err_issue_s(ERR_EXPR_ADDRREG);
                else
                    p->u.sym->f.addressed = 1;
            }
            break;
        case '+':
            lex_tc = lex_next();
            p = tree_pos_s(expr_unary(lev), NULL);
            break;
        case '-':
            lex_tc = lex_next();
            p = tree_neg_s(expr_unary(lev), NULL);
            break;
        case '~':
            lex_tc = lex_next();
            p = tree_bcom_s(expr_unary(lev), NULL);
            break;
        case '!':
            lex_tc = lex_next();
            p = tree_not_s(expr_unary(lev), NULL);
            break;
        case LEX_INCR:
        case LEX_DECR:
            lex_tc = lex_next();
            p = tree_casgn_s(tc, expr_unary(lev), tree_sconst_s(1, ty_inttype));
            break;
        case LEX_SIZEOF:
            err_entersite(lex_cpos);    /* enters with sizeof */
            lex_tc = lex_next();
            {
                ty_t *ty;

                p = NULL;
                if (lex_tc == '(') {
                    lex_tc = lex_next();
                    if (lex_istype()) {
                        ty = decl_typename();
                        err_expect(')');
                    } else {
                        if (lev == TL_PARENE_STD) {
                            err_issuep(lex_ppos, ERR_PARSE_MANYPE);
                            err_issuep(lex_ppos, ERR_PARSE_MANYPESTD, (long)TL_PARENE_STD);
                        }
                        p = expr_postfix(expr_expr(')', lev+1, 0));
                        ty = (p)? p->type: NULL;
                    }
                } else {
                    p = expr_unary(lev);
                    ty = (p)? p->type: NULL;
                }
                if (ty) {
                    if (TY_ISFUNC(ty) || ty->size == 0) {
                        if (p)
                            err_issuep(&p->pos, ERR_EXPR_SIZEOFINV);
                        else
                            err_issue_s(ERR_EXPR_SIZEOFINV);
                    } else if (p && tree_rightkid(p)->op == OP_FIELD)
                        err_issuep(&tree_rightkid(p)->pos, ERR_EXPR_SIZEOFBIT);
                    p = tree_uconst_s(ty->size, ty_sizetype);
                }
            }
            err_exitsite();    /* exits from sizeof */
            break;
        case '(':
            lex_tc = lex_next();
            if (lex_istype()) {    /* cast */
                ty_t *ty, *pty;
                err_entersite(lex_cpos);    /* enters with type name */
                ty = decl_typename();
                ty = TY_UNQUAL(ty);
                err_expect(')');
                p = expr_unary(lev);
                if (p) {
                    p = enode_value_s(enode_pointer_s(p));
                    pty = TY_UNQUAL(p->type);
                    if ((TY_ISARITH(pty) && TY_ISARITH(ty)) || (TY_ISPTR(pty) && TY_ISPTR(ty))) {
                        p = enode_cast_s(p, ty, ENODE_FECAST);
                        if (op_generic(p->op) == OP_CNST) {
                            if (TY_ISFP(ty))
                                p->f.npce |= TREE_FICE;
                            else if (TY_ISPTR(ty))
                                p->f.npce |= (TREE_FACE|TREE_FICE);
                        }
                    } else if ((TY_ISPTR(pty) && TY_ISINTEGER(ty)) ||
                            (TY_ISINTEGER(pty) && TY_ISPTR(ty))) {
                        int npce = 0;
                        if (TY_ISPTR(pty) || !enode_isnpc_s(p)) {
                            err_issue_s(ERR_EXPR_PTRINT);
                            npce = (TREE_FACE|TREE_FICE);
                            if (TY_ISPTR(pty))
                                npce |= TREE_FADDR;
                        } else if (!(TY_ISPTR(ty) && ty->type->op == TY_VOID))
                            npce = (TREE_FACE|TREE_FICE);
                        p = enode_cast_s(p, ty, ENODE_FECAST);
                        if (op_generic(p->op) == OP_CNST && npce)
                            p->f.npce |= npce;
                    } else if (ty->t.type != ty_voidtype) {
                        err_issue_s(ERR_EXPR_INVCAST, pty, ty);
                        p = NULL;
                    }
                    p = (p == NULL || op_generic(p->op) == OP_INDIR || ty->t.type == ty_voidtype)?
                            tree_right_s(NULL, p, ty): tree_retype_s(p, ty);
                }
                err_exitsite();    /* exits from type name */
            } else {    /* expression */
                if (lev == TL_PARENE_STD) {
                    err_issuep(lex_ppos, ERR_PARSE_MANYPE);
                    err_issuep(lex_ppos, ERR_PARSE_MANYPESTD, (long)TL_PARENE_STD);
                }
                p = expr_expr(')', lev+1, 0);
                if (p)
                    p->f.paren = 1;
                p = expr_postfix(p);
            }
            break;
        default:
            p = expr_postfix(expr_prim());
            break;
    }

    err_exitsite();    /* exits from unary operator */
    return p;
}


/*
 *  parses a binary expression:
 *      bin-exp:
 *          unary-exp { bin-op unary-exp }
 *  where operator precedence maintained by a table
 */
static tree_t *expr_bin(int k, int lev)
{
    int k1;
    tree_t *p = expr_unary(lev);

    for (k1 = prec[lex_tc]; k1 >= k; k1--)
        while (prec[lex_tc] == k1 && *in_cp != '=') {
            tree_t *r;
            int op = lex_tc;
            err_entersite(lex_cpos);    /* enters with binary operator */
            lex_tc = lex_next();
            r = expr_bin(k1+1, lev);
            p = tree_optree_s[op](tree_oper[op], p, r, NULL);
            err_exitsite();    /* exits from binary operator */
        }

    return p;
}


/*
 *  parses a conditional expression:
 *      cond-exp:
 *          unary-exp [ ? expression : assign-exp ]
 */
static tree_t *expr_cond(int lev)
{
    tree_t *p = expr_bin(4, lev);

    if (lex_tc == '?') {
        tree_t *l, *r;

        err_entersite(lex_cpos);    /* enters with ? */
        lex_tc = lex_next();
        l = expr_expr(':', lev, 0);
        r = expr_cond(lev);
        p = tree_cond_s(p, l, r, NULL);
        err_exitsite();    /* exits from ? */
    }

    return p;
}


/*
 *  parses an assignment expression:
 *      assign-exp:
 *          cond-exp { assign-op cond-exp }
 *  instead of:
 *      assign-exp:
 *          cond-exp
 *          unary-exp { assign-op cond-exp }
 *  and (c?l:r = v) is treated as a semantic error
 */
tree_t *(expr_asgn)(int tok, int lev, int init)
{
    tree_t *p = expr_cond(lev);

    if (lex_tc == '=' || (prec[lex_tc] >= 6 && prec[lex_tc] <= 8) ||
        (prec[lex_tc] >= 11 && prec[lex_tc] <= 13)) {
        int tc = lex_tc;
        err_entersite(lex_cpos);    /* enters with assignment operator */
        lex_tc = lex_next();
        if (tree_oper[tc] == OP_ASGN)
            p = tree_asgn_s(OP_ASGN, p, expr_asgn(0, lev, 0), NULL);
        else {
            err_expect('=');
            p = tree_casgn_s(tc, p, expr_asgn(0, lev, 0));
        }
        err_exitsite();    /* exits from assignment operator */
    }

    if (tok)
        err_test(tok, err_sset_exprasgn);

    if (init && p)
        tree_chkref(p->orgn, 0);

    return p;
}


/*
 *  parses an expression:
 *      expression: assign-exp { , assign-exp }
 */
tree_t *(expr_expr)(int tok, int lev, int init)
{
    tree_t *p = expr_asgn(0, lev, 0);

    while (lex_tc == ',') {
        tree_t *q;
        lex_tc = lex_next();
        if (!lex_extracomma(';', "expression", 0)) {
            err_entersite(lex_cpos);    /* enters with right expression */
            if ((q = expr_asgn(0, lev, 0)) == NULL || !p)
                p = q = NULL;
            if (p) {
                /* folds constants before tree_root_s() */
                if (main_opt()->std != 1 && simp_needconst &&
                    op_generic(p->op) == OP_CNST && op_generic(q->op) == OP_CNST) {
                    if (!OP_ISINT(p->op) || !OP_ISINT(q->op)) {
                        q->f.npce |= TREE_FICE;
                        if (op_type(p->op) > OP_U || op_type(q->op) > OP_U)
                            q->f.npce |= TREE_FACE;
                    }
                    q->f.npce |= (p->f.npce | TREE_FCOMMA);
                    tree_chkused(p->orgn);
                    p = q;    /* CNST, so no need to keep orgn */
                } else {
                    tree_t *r;
                    tree_chkused(p->orgn);
                    r = tree_right_s(p->orgn, q->orgn, NULL);
                    err_emute();    /* orgn is more accurate for diagnostics */
                    p = tree_right_s(tree_root_s(p), q, NULL);
                    err_eunmute();
                    p->orgn = r;
                }
            }
            err_exitsite();    /* exits from right expression */
        }
    }
    if (tok)
        err_test(tok, err_sset_expr);

    if (init && p)
        tree_chkref(p->orgn, 0);

    return p;
}


/*
 *  parses an expression and takes a sub-tree with side-effects;
 *  constitues a separate function since used as a function pointer
 */
tree_t *(expr_expr0)(int tok, int lev)
{
    tree_t *p = expr_expr(tok, lev, 0);

    if (!p)
        return NULL;

    err_entersite(&p->pos);    /* enters with expression */
    p = enode_pointer_s(p);
    tree_chkref(p->orgn, 1);    /* set V; void expression context */
    tree_chkused(p->orgn);
    p = tree_root_s(p);
    err_exitsite();    /* exits from expression */

    return p;
}

/* end of expr.c */
