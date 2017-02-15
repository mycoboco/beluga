/*
 *  tree handling
 */

#ifndef TREE_H
#define TREE_H

#include <stddef.h>       /* NULL */
#include <cbl/arena.h>    /* arena_t */
#ifndef NDEBUG
#include <stdio.h>        /* FILE */
#endif    /* !NDEBUG */


typedef struct tree_t tree_t;    /* used in dag.h */


#include "common.h"
#include "dag.h"
#include "lmap.h"
#include "sym.h"
#include "ty.h"


/* flag values to handle constant expressions */
enum {
    TREE_FICE   = (1 << 0),    /* cannot be ice */
    TREE_FACE   = (1 << 1),    /* cannot be ace */
    TREE_FADDR  = (1 << 2),    /* cannot be address constant */
    TREE_FCOMMA = (1 << 3)     /* contains comma operator */
};

typedef const lmap_t *tree_pos_t[3];    /* tree loci; left, op, right */

/* tree for expressions;
   tree_t pointer not const-qualified even when read only because frequently contained as members
   of another tree */
struct tree_t {
    int op;                   /* operation (OP_*) */
    ty_t *type;               /* type */
    struct tree_t *kid[3];    /* children; third for diagnostic purpose */
    dag_node_t *node;         /* generated dag */
    tree_pos_t *pos;          /* tree loci */
    struct {
        unsigned ecast:   1;    /* distinguishes explicit casts */
        unsigned eindir:  1;    /* distinguishes explicit indirection */
        unsigned npce:    4;    /* detects non-portable constant expression */
        unsigned cvfpu:   1;    /* detects conversion from fp to uint/ulong */
        unsigned rooted:  1;    /* true if tree_root() applied */
        unsigned checked: 1;    /* true if symbol reference checked */
        unsigned nlval:   1;    /* true if non-lvalue tree */
        unsigned nlvala:  1;    /* true if non-lvalue array warned */
        unsigned paren:   1;    /* true if parenthesized */
    } f;
    union {
        sym_val_t v;           /* value if constant tree */
        sym_t *sym;            /* symbol if used */
        sym_field_t *field;    /* bit-field information */
    } u;
    struct tree_t *orgn;    /* unoptimized tree for diagnostics */
};


extern tree_t *(*tree_optree[])();
extern int tree_oper[];


tree_pos_t *tree_npos(const lmap_t *, const lmap_t *, const lmap_t *);
tree_pos_t *tree_npos1(const lmap_t *);
tree_t *tree_new(int, ty_t *, tree_t *, tree_t *, tree_pos_t *);
tree_t *tree_texpr(tree_t *(*)(int, int, const lmap_t *), int, arena_t *, const lmap_t *);
tree_t *tree_rightkid(tree_t *);
tree_t *tree_root(tree_t *);
tree_t *tree_retype(tree_t *, ty_t *, tree_pos_t *);
int tree_iscallb(const tree_t *);
const char *tree_fname(tree_t *);
tree_t *tree_right(tree_t *, tree_t *, ty_t *, tree_pos_t *);
tree_t *tree_asgn(int, tree_t *, tree_t *, ty_t *, tree_pos_t *);
tree_t *tree_asgnf(int, tree_t *, tree_t *, ty_t *, tree_pos_t *);
tree_t *tree_asgnid(sym_t *, tree_t *, tree_pos_t *);
tree_t *tree_casgn(int, tree_t *, tree_t *, tree_pos_t *);
tree_t *tree_cond(tree_t *, tree_t *, tree_t *, ty_t *, tree_pos_t *);
tree_t *tree_and(int, tree_t *, tree_t *, ty_t *, tree_pos_t *);
tree_t *tree_bit(int, tree_t *, tree_t *, ty_t *, tree_pos_t *);
tree_t *tree_cmp(int, tree_t *, tree_t *, ty_t *, tree_pos_t *);
tree_t *tree_sha(int, tree_t *, tree_t *, ty_t *, tree_pos_t *);
tree_t *tree_sh(int, tree_t *, tree_t *, ty_t *, tree_pos_t *);
tree_t *tree_add(int, tree_t *, tree_t *, ty_t *, tree_pos_t *);
tree_t *tree_sub(int, tree_t *, tree_t *, ty_t *, tree_pos_t *);
tree_t *tree_mul(int, tree_t *, tree_t *, ty_t *, tree_pos_t *);
tree_t *tree_pos(tree_t *, ty_t *, tree_pos_t *);
tree_t *tree_neg(tree_t *, ty_t *, tree_pos_t *);
tree_t *tree_bcom(tree_t *, ty_t *, tree_pos_t *);
tree_t *tree_not(tree_t *, ty_t *, tree_pos_t *);
tree_t *tree_indir(tree_t *, ty_t *, int, tree_pos_t *);
tree_t *tree_addr(tree_t *, ty_t *, int, tree_pos_t *);
tree_t *tree_pcall(tree_t *);
tree_t *tree_dot(int, tree_t *);
tree_t *tree_sconst(sx_t, ty_t *, tree_pos_t *);
tree_t *tree_uconst(ux_t, ty_t *, tree_pos_t *);
tree_t *tree_fpconst(long double, ty_t *, tree_pos_t *);
tree_t *tree_id(sym_t *, tree_pos_t *);
tree_t *tree_untype(tree_t *);
void tree_chkref(tree_t *, unsigned);
int tree_chkused(tree_t *);
int tree_chkinit(const tree_t *);
#ifndef NDEBUG
int tree_pnodeid(const void *p);
int *tree_printed(int id);
void tree_printnew(void);
void tree_print(const tree_t *, FILE *);
#endif    /* !NDEBUG */


/* ensures rvalue-ness */
#define TREE_ISLVAL(p) (op_generic((p)->op) == OP_INDIR || op_generic((p)->op) == OP_FIELD)
#define TREE_RVAL(p, pos)                                     \
    ((TREE_ISLVAL(p))? tree_right(NULL, (p), NULL, (pos)):    \
                       tree_retype((p), NULL, (pos)))

/* converts tree loci to a source locus;
   P: tree_pos_t, T: tree_t, N: nullable tree_t */
#define TREE_PW(p) (lmap_range((*(p))[0], (*(p))[2]))
#define TREE_PL(p) ((*(p))[0])
#define TREE_PO(p) ((*(p))[1])
#define TREE_PR(p) ((*(p))[2])
#define TREE_TW(p) (TREE_PW((p)->orgn->pos))
#define TREE_TL(p) (TREE_PL((p)->orgn->pos))
#define TREE_TO(p) (TREE_PO((p)->orgn->pos))
#define TREE_TR(p) (TREE_PR((p)->orgn->pos))
#define TREE_NW(p) ((p)? TREE_PW((p)->orgn->pos): NULL)
#define TREE_NL(p) ((p)? TREE_PL((p)->orgn->pos): NULL)
#define TREE_NO(p) ((p)? TREE_PO((p)->orgn->pos): NULL)
#define TREE_NR(p) ((p)? TREE_PR((p)->orgn->pos): NULL)


#endif    /* TREE_H */

/* end of tree.h */
