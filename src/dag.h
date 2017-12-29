/*
 *  expression dag
 */

#ifndef DAG_H
#define DAG_H

#include <stddef.h>    /* NULL */
#ifndef NDEBUG
#include <stdio.h>     /* FILE */
#endif    /* !NDEBUG */


typedef struct dag_node_t dag_node_t;    /* used in sym.h */


#include "sym.h"
#include "tree.h"

#include "cfg.h"


/* dag node */
struct dag_node_t {
    int op;                       /* dag operation code */
    short count;                  /* # of references as child */
    sym_t *sym[3];                /* symbols */
    struct dag_node_t *kid[2];    /* children */
    struct dag_node_t *link;      /* next root node */
    unsigned usecse: 1;           /* forces use of cse if set */

    cfg_node_t x;                 /* extension for back-end */
};


extern int dag_nodecount;    /* # of nodes in hash buckets */


dag_node_t *dag_newnode(int, dag_node_t *, dag_node_t *, sym_t *);
dag_node_t *dag_listnode(tree_t *, int, int);
void dag_walk(tree_t *, int, int);
void dag_gencode(void *[], void *[]);    /* sym_t */
void dag_emitcode(void);
dag_node_t *dag_copy(const dag_node_t *);
#ifndef NDEBUG
void dag_print(const dag_node_t *, FILE *, int);
#endif    /* !NDEBUG */


/* for-performance wrapper of dag_listnode();
   dag_listnode() used when no check is necessary;
   returning tp->node moved into dag_listnode() to set usecse */
#define DAG_LISTNODE(tp, tlab, flab) ((!(tp))? NULL: dag_listnode(tp, tlab, flab))


#endif    /* DAG_H */

/* end of dag.h */
