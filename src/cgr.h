/*
 *  code generation rule
 */

#ifndef CGR_H
#define CGR_H

#ifndef NDEBUG
#include <stdio.h>    /* FILE */
#endif    /* !NDEBUG */

#include "alist.h"
#include "dag.h"
#include "op.h"

#define CGR_CSTMAX     1000                  /* max. value of cost */
#define cgr_ntidx(nt)  ((nt) - (OP_2+1))     /* non-terminal to index */
#define cgr_idxnt(idx) ((idx) + (OP_2+1))    /* index to non-terminal */
#define cgr_isnt(op)   ((op) > OP_2)         /* true if op is non-terminal */


/* tree representation of BURS rules */
typedef struct cgr_tree_t {
    int op;                       /* op code + non-terminal */
    struct cgr_tree_t *kid[2];    /* children */
} cgr_tree_t;

/* BURS rule */
typedef struct cgr_t {
    short rn;                      /* rule number */
    int nt;                        /* non-terminal */
    const int *ot;                 /* array representation */
    short cost;                    /* cost if costf is NULL */
    int (*costf)(dag_node_t *);    /* cost function if not NULL */
    const char *tmpl;              /* asm template */
    cgr_tree_t *tree;              /* tree representation */
    unsigned char nnt;             /* # of non-terminals in rule */
    unsigned char isinst;          /* true if instruction */
} cgr_t;


alist_t *cgr_lookup(int);
cgr_t *cgr_add(cgr_t *);
cgr_tree_t *cgr_tree(const int *, unsigned char *);
#ifndef NDEBUG
void cgr_tmpl(const char *, FILE *);
void cgr_print(const cgr_t *, FILE *);
#endif    /* !NDEBUG */


#endif    /* CGR_H */

/* end of cgr.h */
