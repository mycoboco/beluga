/*
 *  experssion parsing
 */

#include <stddef.h>        /* NULL */
#include <cbl/assert.h>    /* assert */
#include <cdsl/hash.h>     /* hash_new */

#include "clx.h"
#include "decl.h"
#include "enode.h"
#include "err.h"
#include "ir.h"
#include "lex.h"
#include "lmap.h"
#include "main.h"
#include "op.h"
#include "sset.h"
#include "simp.h"
#include "stmt.h"
#include "strg.h"
#include "sym.h"
#include "tree.h"
#include "ty.h"
#include "expr.h"


double expr_refinc = 1.0;    /* weight for reference counter */


/* operator precedence */
static char prec[] = {
#define xx(a, b, c, d, e, f, g, h) c,
#define kk(a, b, c, d, e, f, g, h) c,
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

    assert(clx_tc != '(');

    switch(clx_tc) {
        case LEX_CCON:
        case LEX_ICON:
        case LEX_FCON:
            if (clx_sym) {
                p = tree_new(OP_CNST+op_sfx(clx_sym->type), clx_sym->type, NULL, NULL,
                             tree_npos1(clx_cpos));
                p->u.v = clx_sym->u.c.v;
            } else
                p = NULL;
            break;
        case LEX_SCON:
            clx_sym->u.c.v.hp = hash_new(clx_sym->u.c.v.hp, clx_sym->type->size);
            clx_sym = sym_findconst(clx_sym->type, clx_sym->u.c.v);
            if (!clx_sym->u.c.loc)
                clx_sym->u.c.loc = sym_new(SYM_KGEN, LEX_STATIC, clx_sym->type, SYM_SGLOBAL);
            clx_sym->u.c.loc->f.outofline = 1;
            p = tree_id(clx_sym->u.c.loc, tree_npos1(clx_cpos));
            break;
        case LEX_ID:
            if (!clx_sym) {
                sym_t *q = sym_new(SYM_KORDIN, clx_tok, clx_cpos,
                                   LEX_AUTO, ty_inttype,    /* modified later */
                                   (sym_scope < SYM_SPARAM)? strg_perm: strg_func);
                clx_tc = clx_next();
                if (clx_tc == '(') {
                    sym_t *r = sym_lookup(clx_tok, sym_extern);
                    const lmap_t *pos = lmap_range(q->pos, clx_cpos);
                    q->sclass = LEX_EXTERN;
                    q->type = ty_func(ty_inttype, NULL, 1, pos);
                    (void)(err_dpos(pos, ERR_EXPR_IMPLDECL) &&
                           err_dpos(pos, ERR_EXPR_IMPLDECLSTD));
                    err_dpos(pos, ERR_PARSE_NOPROTO, q->name, " function");
                    if (!decl_chkid(clx_tok, q->pos, sym_global, 1))
                        decl_chkid(clx_tok, q->pos, sym_extern, 1);
                    if (r && !ty_equiv(q->type, r->type, 1))
                        (void)(err_dpos(pos, ERR_PARSE_REDECLTYW, q, " an identifier",
                                        q->type, r->type) &&
                               err_dpos(r->pos, ERR_PARSE_PREVDECL));
                    ir_cur->symgsc(q);
                    if (!r)
                        sym_new(SYM_KEXTERN, q->name, q->pos, LEX_EXTERN, q->type);
                    p = tree_id(q, tree_npos1(clx_ppos));
                } else {
                    err_dpos(q->pos, ERR_EXPR_NOID, q, "");
                    q->type = ty_unknowntype;
                    if (q->scope == SYM_SGLOBAL)
                        ir_cur->symgsc(q);
                    else
                        stmt_local(q);
                    p = NULL;
                }
                assert(q->type && q->sclass);
                if (main_opt()->xref)
                    sym_use(clx_sym, clx_cpos);
                return p;
            }
            if (main_opt()->xref)
                sym_use(clx_sym, clx_cpos);
            if (clx_sym->sclass == LEX_ENUM)
                p = tree_sconst(clx_sym->u.value, ty_inttype, tree_npos1(clx_cpos));
            else if (!TY_ISUNKNOWN(clx_sym->type)) {
                if (clx_sym->sclass == LEX_TYPEDEF) {
                    err_dpos(clx_cpos, ERR_EXPR_ILLTYPEDEF, clx_sym, "");
                    p = NULL;
                } else
                    p = tree_id(clx_sym, tree_npos1(clx_cpos));
            } else
                p = NULL;
            break;
        default:
            err_dpos(lmap_after(clx_ppos), ERR_EXPR_ILLEXPR);
            if (!tree_optree[clx_tc])    /* for better diagnostics */
                switch(clx_tc) {
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
                        clx_tc = clx_next();
                }
            return NULL;
    }
    clx_tc = clx_next();

    return p;
}


/*
 *  parses a postfix expression:
 *      postfix-exp:
 *          primary-exp { postfix-op }
 */
static tree_t *expr_postfix(tree_t *p)
{
    tree_t *q;
    tree_pos_t *tpos;
    const lmap_t *pos;

    assert(ty_inttype);    /* ensures types initialized */

    for (;;) {
        pos = clx_cpos;
        switch(clx_tc) {
            case LEX_INCR:
            case LEX_DECR:
                tpos = tree_npos(TREE_NL(p), pos, pos);
                q = tree_casgn(clx_tc, p, tree_sconst(1, ty_inttype, tpos), tpos);
                p = (q)? tree_right(tree_right(p, q, NULL, tpos), p, NULL, tpos): NULL;
                clx_tc = clx_next();
                break;
            case '[':
                clx_tc = clx_next();
                q = expr_expr(']', 0, 0, pos);
                if (q && TY_UNQUAL(q->type)->t.type == ty_chartype && op_generic(q->op) != OP_CNST)
                    err_dpos(TREE_TW(q), ERR_EXPR_CHARSUBSCR);
                tpos = tree_npos(TREE_NL(p), pos, clx_ppos);
                p = tree_indir(tree_optree['+'](OP_SUBS, p, q, NULL, tpos), NULL, 1, tpos);
                break;
            case '(':
                /* no call to clx_next() to let tree_pcall() do */
                p = tree_pcall(p);
                break;
            case '.':
            case LEX_DEREF:
                /* no call to clx_next() to let tree_dot() do */
                p = tree_dot(clx_tc, p);
                break;
            default:
                return p;
        }
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
    tree_pos_t *tpos;
    const lmap_t *pos, *posm;

    assert(ty_inttype);    /* ensures types initialized */

    pos = clx_cpos;
    switch(tc = clx_tc) {
        case '*':
        case '&':
        case '+':
        case '-':
        case '~':
        case '!':
        case LEX_INCR:
        case LEX_DECR:
            {
                tree_t *q;

                clx_tc = clx_next();
                q = expr_unary(lev);
                tpos = tree_npos(pos, pos, clx_ppos);
                switch(tc) {
                    case '*':
                        p = tree_indir(q, NULL, 1, tpos);
                        break;
                    case '&':
                        p = tree_addr(q, NULL, 1, tpos);
                        /* checked here to allow assignment to temporary object */
                        if (p) {
                            if (OP_ISADDR(p->op)) {
                                if (p->u.sym->sclass == LEX_REGISTER)
                                    err_dmpos(pos, ERR_EXPR_ADDRREG, TREE_TW(q), NULL);
                                else
                                    p->u.sym->f.addressed = 1;
                            } else if (op_generic(p->op) == OP_INDIR)
                                p = tree_right(NULL, p, NULL, tpos);    /* rvalue */
                        }
                        break;
                    case '+':
                        p = tree_pos(q, NULL, tpos);
                        break;
                    case '-':
                        p = tree_neg(q, NULL, tpos);
                        break;
                    case '~':
                        p = tree_bcom(q, NULL, tpos);
                        break;
                    case '!':
                        p = tree_not(q, NULL, tpos);
                        break;
                    case LEX_INCR:
                    case LEX_DECR:
                        p = tree_casgn(tc, q, tree_sconst(1, ty_inttype, tpos), tpos);
                        break;
                    default:
                        assert(!"invalid operator -- should never reach here");
                        break;
                }
            }
            break;
        case LEX_SIZEOF:
            clx_tc = clx_next();
            {
                ty_t *ty;

                p = NULL;
                if (clx_tc == '(') {
                    posm = clx_cpos;
                    clx_tc = clx_next();
                    if (clx_istype(CLX_TYLAS)) {
                        ty = decl_typename(CLX_TYLAS);
                        sset_expect(')', posm);
                    } else {
                        if (lev == TL_PARENE_STD)
                            (void)(err_dpos(clx_ppos, ERR_PARSE_MANYPE) &&
                                   err_dpos(clx_ppos, ERR_PARSE_MANYPESTD, (long)TL_PARENE_STD));
                        p = expr_postfix(expr_expr(')', lev+1, 0, posm));
                        ty = (p)? p->type: NULL;
                    }
                } else {
                    p = expr_unary(lev);
                    ty = (p)? p->type: NULL;
                }
                if (ty && !TY_ISUNKNOWN(ty)) {
                    if (ty->size == 0) {
                        err_dmpos(pos, (TY_ISFUNC(ty))? ERR_EXPR_SIZEOFFUNC: ERR_EXPR_SIZEOFINC,
                                  (p)? TREE_TW(p): lmap_range(posm, clx_ppos), NULL);
                        p = NULL;
                    } else if (p && tree_rightkid(p)->op == OP_FIELD) {
                        err_dmpos(pos, ERR_EXPR_SIZEOFBIT, TREE_TW(p), NULL);
                        p = NULL;
                    } else
                        p = tree_uconst(ty->size, ty_sizetype, tree_npos(pos, pos, clx_ppos));
                }
            }
            break;
        case '(':
            posm = pos;
            clx_tc = clx_next();
            if (clx_istype(CLX_TYLAC)) {    /* cast */
                ty_t *ty, *pty;
                ty = decl_typename(CLX_TYLAC);
                ty = TY_UNQUAL(ty);
                sset_expect(')', posm);
                pos = lmap_range(posm, clx_ppos);
                p = expr_unary(lev);
                if (TY_ISUNKNOWN(ty))
                    p = NULL;
                if (p) {
                    tpos = tree_npos(pos, pos, TREE_NR(p));
                    p = enode_value(enode_pointer(p));
                    pty = TY_UNQUAL(p->type);
                    if ((TY_ISARITH(pty) && TY_ISARITH(ty)) || (TY_ISPTR(pty) && TY_ISPTR(ty))) {
                        p = enode_cast(p, ty, 0, pos);
                        if (op_generic(p->op) == OP_CNST) {
                            if (TY_ISFP(ty))
                                p->f.npce |= TREE_FICE;
                            else if (TY_ISPTR(ty))
                                p->f.npce |= (TREE_FACE|TREE_FICE);
                        }
                    } else if ((TY_ISPTR(pty) && TY_ISINTEGER(ty)) ||
                               (TY_ISINTEGER(pty) && TY_ISPTR(ty))) {
                        int npce = 0;
                        if (TY_ISPTR(pty) || !enode_isnpc(p)) {
                            err_dmpos(pos, ERR_EXPR_PTRINT, TREE_TW(p), NULL);
                            npce = (TREE_FACE|TREE_FICE);
                            if (TY_ISPTR(pty))
                                npce |= TREE_FADDR;
                        } else if (!(TY_ISPTR(ty) && ty->type->op == TY_VOID))
                            npce = (TREE_FACE|TREE_FICE);
                        p = enode_cast(p, ty, 0, pos);
                        if (op_generic(p->op) == OP_CNST && npce)
                            p->f.npce |= npce;
                    } else if (ty->t.type != ty_voidtype) {
                        err_dmpos(pos, ERR_EXPR_INVCAST, TREE_TW(p), NULL, pty, ty);
                        p = NULL;
                    }
                    if (p) {
                        p = (op_generic(p->op) == OP_INDIR || ty->t.type == ty_voidtype)?
                                tree_right(NULL, p, ty, tpos): tree_retype(p, NULL, tpos);
                        p->orgn->f.ecast = 1;    /* tree_chkused() invoked with orgn */
                    }
                }
            } else {    /* expression */
                if (lev == TL_PARENE_STD)
                    (void)(err_dpos(clx_ppos, ERR_PARSE_MANYPE) &&
                           err_dpos(clx_ppos, ERR_PARSE_MANYPESTD, (long)TL_PARENE_STD));
                p = expr_expr(')', lev+1, 0, posm);
                if (p) {
                    p->orgn->f.paren = 1;    /* paren checked through orgn */
                    p->orgn->pos = tree_npos(posm, TREE_TO(p), clx_ppos);    /* includes parens */
                }
                p = expr_postfix(p);
            }
            break;
        default:
            p = expr_postfix(expr_prim());
            break;
    }

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

    for (k1 = prec[clx_tc]; k1 >= k; k1--)
        while (prec[clx_tc] == k1) {
            tree_t *r;
            int op = clx_tc;
            const lmap_t *pos = clx_cpos;
            clx_tc = clx_next();
            r = expr_bin(k1+1, lev);
            p = tree_optree[op](tree_oper[op], p, r, NULL, tree_npos(TREE_NL(p), pos, TREE_NR(r)));
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

    if (clx_tc == '?') {
        tree_t *l, *r;
        const lmap_t *pos = clx_cpos;    /* ? */

        clx_tc = clx_next();
        l = expr_expr(':', lev, 0, pos);
        r = expr_cond(lev);
        p = tree_cond(p, l, r, NULL, tree_npos(TREE_NL(p), pos, TREE_NR(r)));
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
tree_t *(expr_asgn)(int tok, int lev, int init, const lmap_t *posm)
{
    tree_t *p = expr_cond(lev), *q;

    if (prec[clx_tc] == 2) {
        int tc = clx_tc;
        tree_pos_t *tpos;
        const lmap_t *pos = clx_cpos;

        clx_tc = clx_next();
        q = expr_asgn(0, lev, 0, posm);
        tpos = tree_npos(TREE_NL(p), pos, TREE_NR(q));
        p = (tree_oper[tc] == OP_ASGN)?
                tree_asgn(OP_ASGN, p, q, NULL, tpos): tree_casgn(tc, p, q, tpos);
    }

    if (tok)
        sset_test(tok, sset_exprasgn, posm);

    if (init && p)
        tree_chkref(p->orgn, 0);

    return p;
}


/*
 *  parses an expression:
 *      expression: assign-exp { , assign-exp }
 */
tree_t *(expr_expr)(int tok, int lev, int init, const lmap_t *posm)
{
    tree_t *p = expr_asgn(0, lev, 0, posm);

    while (clx_tc == ',') {
        tree_t *q;
        clx_tc = clx_next();
        if (!clx_xtracomma(';', "expression", 0)) {
            const lmap_t *pos = clx_cpos;
            if ((q = expr_asgn(0, lev, 0, posm)) == NULL || !p)
                p = q = NULL;
            if (p) {
                /* folds constants before tree_root() */
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
                    tree_pos_t *tpos = tree_npos(TREE_TL(p), pos, TREE_TR(q));
                    tree_chkused(p->orgn);
                    r = tree_right(p->orgn, q->orgn, NULL, tpos);
                    err_mute();    /* orgn is more accurate for diagnostics */
                    p = tree_right(tree_root(p), q, NULL, tpos);
                    err_unmute();
                    p->orgn = r;
                }
            }
        }
    }
    if (tok)
        sset_test(tok, sset_expr, posm);

    if (init && p)
        tree_chkref(p->orgn, 0);

    return p;
}


/*
 *  parses an expression and takes a sub-tree with side-effects;
 *  constitues a separate function since used as a function pointer
 */
tree_t *(expr_expr0)(int tok, int lev, const lmap_t *posm)
{
    tree_t *p = expr_expr(tok, lev, 0, posm);

    if (!p)
        return NULL;

    p = enode_pointer(p);
    tree_chkref(p->orgn, 1);    /* set V; void expression context */
    tree_chkused(p->orgn);
    p = tree_root(p);

    return p;
}

/* end of expr.c */
