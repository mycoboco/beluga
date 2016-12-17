/*
 *  tree handling
 */

#ifndef TREE_H
#define TREE_H

#include <cbl/arena.h>    /* arena_t */
#ifndef NDEBUG
#include <stdio.h>        /* FILE */
#endif    /* !NDEBUG */


typedef struct tree_t tree_t;    /* used in dag.h */


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

/* tree for expressions;
   tree_t pointer not const-qualified even when read only because frequently contained as members
   of another tree */
struct tree_t {
    int op;                   /* operation (OP_*) */
    ty_t *type;               /* type */
    struct tree_t *kid[3];    /* children; third for diagnostic purpose */
    dag_node_t *node;         /* generated dag */
    const lmap_t *pos;        /* locus */
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


tree_t *tree_new(int, ty_t *, tree_t *, tree_t *, const lmap_t *);
tree_t *tree_texpr(tree_t *(*)(int, int), int, arena_t *);
tree_t *tree_rightkid(tree_t *);
tree_t *tree_root(tree_t *, const lmap_t *);
tree_t *tree_retype(tree_t *, ty_t *, const lmap_t *);
int tree_iscallb(const tree_t *);
const char *tree_fname(tree_t *);
tree_t *tree_right(tree_t *, tree_t *, ty_t *, const lmap_t *);
tree_t *tree_asgn(int, tree_t *, tree_t *, ty_t *, const lmap_t *);
tree_t *tree_asgnf(int, tree_t *, tree_t *, ty_t *, const lmap_t *);
tree_t *tree_asgnid(sym_t *, tree_t *, const lmap_t *);
tree_t *tree_casgn(int, tree_t *, tree_t *, const lmap_t *);
tree_t *tree_cond(tree_t *, tree_t *, tree_t *, ty_t *, const lmap_t *);
tree_t *tree_and(int, tree_t *, tree_t *, ty_t *, const lmap_t *);
tree_t *tree_bit(int, tree_t *, tree_t *, ty_t *, const lmap_t *);
tree_t *tree_cmp(int, tree_t *, tree_t *, ty_t *, const lmap_t *);
tree_t *tree_sha(int, tree_t *, tree_t *, ty_t *, const lmap_t *);
tree_t *tree_sh(int, tree_t *, tree_t *, ty_t *, const lmap_t *);
tree_t *tree_add(int, tree_t *, tree_t *, ty_t *, const lmap_t *);
tree_t *tree_sub(int, tree_t *, tree_t *, ty_t *, const lmap_t *);
tree_t *tree_mul(int, tree_t *, tree_t *, ty_t *, const lmap_t *);
tree_t *tree_pos(tree_t *, ty_t *, const lmap_t *);
tree_t *tree_neg(tree_t *, ty_t *, const lmap_t *);
tree_t *tree_bcom(tree_t *, ty_t *, const lmap_t *);
tree_t *tree_not(tree_t *, ty_t *, const lmap_t *);
tree_t *tree_indir(tree_t *, ty_t *, int, const lmap_t *);
tree_t *tree_addr(tree_t *, ty_t *, int, const lmap_t *);
tree_t *tree_pcall(tree_t *);
tree_t *tree_dot(int, tree_t *, const lmap_t *);
tree_t *tree_sconst(long, ty_t *, const lmap_t *);
tree_t *tree_uconst(unsigned long, ty_t *, const lmap_t *);
tree_t *tree_fpconst(long double, ty_t *, const lmap_t *);
tree_t *tree_id(sym_t *, const lmap_t *);
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


#endif    /* TREE_H */

/* end of tree.h */
