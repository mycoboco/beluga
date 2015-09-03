/*
 *  context stack for preprocessor
 */

#ifndef CTX_H
#define CTX_H

#include <stdio.h>    /* FILE */


#include "lxl.h"


/* context types */
enum {
    CTX_TNORM,     /* retrieves token; touch etab */
    CTX_TPEEK,     /* retrieves token; no change */
    CTX_TIGNORE    /* retrieves token; set ignore */
};


/* context stack */
typedef struct ctx_t {
    char type;             /* context type */
    lxl_t *list;           /* lexical list; see exparg() from mcr.c */
    lxl_node_t *cur;       /* current node */
    struct ctx_t *prev;    /* previous context */
    struct ctx_t *next;    /* next context */
} ctx_t;


ctx_t *ctx_cur;    /* current context */


int ctx_isbase(void);
ctx_t *ctx_push(int);
void ctx_pop(void);
void ctx_init(void);
void ctx_print(FILE *);


#endif    /* CTX_H */

/* end of ctx.h */
