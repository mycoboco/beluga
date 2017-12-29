/*
 *  register
 */

#ifndef REG_H
#define REG_H

#include <cbl/arena.h>    /* arena_t */

#include "common.h"
#include "dag.h"
#include "op.h"
#include "sym.h"


/* register sets */
enum {
    REG_SINT,    /* general purpose */
    REG_SFP,     /* floating-point */
    REG_SMAX
};

/* register symbol index */
enum {
    REG_RX = NELEM(((dag_node_t *)0)->sym)-1
};

/* register mask */
typedef unsigned long reg_mask_t;


extern reg_mask_t *reg_fmask[REG_SMAX],    /* free register masks */
                  *reg_umask[REG_SMAX],    /* used register masks */
                  *reg_tmask[REG_SMAX],    /* temporary register masks */
                  *reg_vmask[REG_SMAX];    /* register variable masks */


reg_mask_t *reg_mask(arena_t *, int, ...);
void reg_mclear(reg_mask_t *);
void reg_mfill(reg_mask_t *);
reg_mask_t *reg_mbackup(const reg_mask_t *);
reg_mask_t *reg_mrestore(reg_mask_t *, const reg_mask_t *);
sym_t *reg_new(const char *, int, int, ...);
sym_t *reg_wildcard(sym_t *[]);
int reg_askvar(sym_t *, sym_t *);
int reg_shares(const sym_t *, const sym_t *);
void reg_pmset(dag_node_t *, const sym_t *);
void reg_pmask(dag_node_t *);
void reg_spill(const reg_mask_t *, int, dag_node_t *);
void reg_alloc(dag_node_t *);
void reg_set(dag_node_t *, sym_t *);
void reg_setnt(dag_node_t *, sym_t *);
void reg_target(dag_node_t *, int, sym_t *);
void reg_ptarget(dag_node_t *, int, sym_t *);


/* checks if symbol/register is allocated for variable */
#define REG_ISRVAR(s)     ((s)->x.regnode->vbl)
#define REG_FORRVAR(s, t) ((s)->x.regnode->vbl == (t))

/* checks if dag node needs register;
   ASSUMPTION: CALL and LOAD are only ops that can take registers on root */
#define REG_ROOT(p) (op_generic((p)->op) == OP_CALL || op_generic((p)->op) == OP_LOAD)
#define REG_NEED(p) (!(p)->x.f.listed || REG_ROOT(p))


#endif    /* REG_H */

/* end of reg.h */
