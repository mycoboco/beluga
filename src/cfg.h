/*
 *  extensions for back-end
 */

#ifndef CFG_H
#define CFG_H

#include "common.h"    /* no other #includes */


struct dag_node_t;
struct sym_t;
struct cgr_t;

/* dag node extension (for dag_node_t) */
typedef struct cfg_node_t {
    short *rn;                     /* nt index to rule number */
    short *cost;                   /* nt index to cost */
    struct dag_node_t **vr;        /* children to reduce */
    struct dag_node_t *kid[3];     /* children for register allocation */
    short inst;                    /* index from NT if instruction */
    struct dag_node_t *prev;       /* previous node in exec order */
    struct dag_node_t *next;       /* next node in exec order */
    struct dag_node_t *prevuse;    /* previous use of cse */
    void *pmask;                   /* (reg_mask_t) precluding mask; to avoid reg.h */
    struct {
        unsigned listed:     1;    /* true if root */
        unsigned spill:      1;    /* true if genspill node */
        unsigned copy:       1;    /* true if register-to-register copy */
        unsigned equatable:  1;    /* true if register-to-cse copy */
        unsigned registered: 1;    /* true if register allocated */
        unsigned emitted:    1;    /* true if emitted */
    } f;
} cfg_node_t;

/* symbol extension (for sym_t) */
typedef struct cfg_sym_t {
    const char *name;    /* name used */
    ssz_t offset;         /* frame offset for param/local */
    struct {
        unsigned exported: 1;    /* symbol exported */
        unsigned imported: 1;    /* symbol imported */
    } f;
    struct {
        short set;            /* register set; REG_S* */
        short num;            /* register number */
        void *bv;             /* (reg_mask_t) bit-vector; to avoid reg.h */
        struct sym_t *vbl;    /* reference to symbol for register variable */
    } *regnode;               /* register */
    int usecnt;                    /* */
    struct sym_t **wildcard;       /* register wildcard */
    struct dag_node_t *lastuse;    /* last use of cse */
} cfg_sym_t;

/* interface extension (for ir_t) */
typedef struct cfg_ir_t {
    char fmt;              /* format prefix for emitter; cannot be ? or # */
    short nreg;            /* # of registers */
    short nnt;             /* # of non-terminals */
    struct cgr_t *rule;    /* rules */
    const char *(*ntname)(int);                 /* returns name of a non-terminal */
    struct sym_t *(*rmapw)(int);                /* returns wildcard for ty/scode */
    int (*rmaps)(int);                          /* returns register set for ty/scode */
    void (*prerewrite)(struct dag_node_t *);    /* prepares rewrite */
    void (*target)(struct dag_node_t *);        /* targets register */
    void (*clobber)(struct dag_node_t *);       /* clobbers registers */
    void (*emit)(struct dag_node_t *);          /* emits target-specific output */
} cfg_ir_t;


#endif    /* CFG_H */

/* end of cfg.h */
