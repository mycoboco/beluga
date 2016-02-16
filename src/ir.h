/*
 *  IR interface and binding
 */

#ifndef IR_H
#define IR_H

#include "cfg.h"
#include "cgr.h"
#include "dag.h"
#include "gen.h"
#include "sym.h"


/* IR interface */
typedef struct ir_t {
    struct {
        unsigned char size,         /* size of type in byte */
                      align,        /* alignment restiction of type */
                      outofline;    /* true if constant of type cannot appear in instruction */
    } charmetric,       /* type metric for plain/signed/unsigned char */
      shortmetric,      /* type metric for signed/unsigned short */
      intmetric,        /* type metric for signed/unsigned int */
      longmetric,       /* type metric for signed/unsigned long */
      floatmetric,      /* type metric for float */
      doublemetric,     /* type metric for double */
      ldoublemetric,    /* type metric for long double */
      ptrmetric,        /* type metric for pointer */
      structmetric;     /* type metric for struct */
    struct {
        unsigned little_endian: 1;    /* true if byte ordering is little-endian */
        unsigned little_bit: 1;       /* true if bit ordering is little-endian */
        unsigned want_callb: 1;       /* true if CALLB should be used */
        unsigned want_argb: 1;        /* true if ARGB should be used */
        unsigned left_to_right: 1;    /* true if arguments passed from left to right */
        unsigned want_dag: 1;         /* true if no need to undag */
    } f;
    FILE *out;    /* output file */

    void (*symaddr)(sym_t *, sym_t *, long);               /* sets x of address symbol */
    void (*symgsc)(sym_t *);                               /* sets x of global/static/constant */
    void (*symlocal)(sym_t *);                             /* sets x of local */
    void (*option)(int *, char **[],
                   void (*)(const char *, ...));           /* handles back-end option */
    void (*progbeg)(FILE *);                               /* initializes program */
    void (*progend)(void);                                 /* finalizes program */
    void (*blockbeg)(gen_env_t *);                         /* initializes block */
    void (*blockend)(const gen_env_t *);                   /* finalizes block */
    void (*defglobal)(sym_t *);                            /* defines global */
    void (*cmpglobal)(sym_t *);                            /* completes global array */
    void (*initaddr)(sym_t *);                             /* provides symbol initializer */
    void (*initconst)(int, sym_val_t);                     /* provides constant initializer */
    void (*initstr)(long, const char *);                   /* provides string initializer */
    void (*initspace)(long);                               /* provides zero-padded initializer */
    void (*export)(sym_t *);                               /* exports global symbol */
    void (*import)(sym_t *);                               /* imports global symbol */
    void (*function)(sym_t *, void *[], void *[], int);    /* (sym_t) processes function */
    void (*emit)(dag_node_t *);                            /* emits code */
    dag_node_t *(*gen)(dag_node_t *);                      /* anotates dag */
    void (*segment)(int);                                  /* changes segment */

    cfg_ir_t x;    /* extension for back-end */
} ir_t;


ir_t *ir_bind(const char *);


extern ir_t *ir_cur;    /* current IR binding */


#endif    /* IR_H */

/* end of ir.h */
