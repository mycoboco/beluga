/*
 *  code generator
 */

#ifndef GEN_H
#define GEN_H

#include <stddef.h>    /* size_t */

#include "dag.h"
#include "reg.h"
#include "sym.h"


/* block environment */
typedef struct {
    int offset;                     /* offset remembered */
    reg_mask_t *fmask[REG_SMAX];    /* register allocation status */
} gen_env_t;


extern int gen_off, gen_maxoff;      /* offset and max offset for locals */
extern int gen_aoff, gen_maxaoff;    /* offset and max offset for arguments */
extern int gen_frame;                /* frame size */


void gen_rewrite(dag_node_t *);
dag_node_t **gen_prune(dag_node_t *, dag_node_t *[], dag_node_t **);
void gen_linearize(dag_node_t *, dag_node_t *);
void gen_wildcard(dag_node_t *);
dag_node_t *gen_code(dag_node_t *);
void gen_blkbeg(gen_env_t *);
void gen_blkend(const gen_env_t *);
void gen_auto(sym_t *, int);
int gen_arg(int, int);
void gen_emit(dag_node_t *);
const char *gen_sfmt(size_t, const char *, ...);

/* common cost functions */
int gen_move(dag_node_t *);
int gen_notarget(dag_node_t *);


/* checks if node reads cse */
#define GEN_READCSE(p) (op_generic((p)->op) == OP_INDIR && (p)->kid[0]->op == OP_VREGP &&    \
                        (p)->sym[REG_RX] && (p)->sym[REG_RX]->u.t.cse)

#define GEN_CSE(p) ((p)->sym[REG_RX]->u.t.cse)    /* original node for cse */


#endif    /* GEN_H */

/* end of gen.h */
