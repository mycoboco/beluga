/*
 *  lexical linked-list for preprocessor
 */

#ifndef LXL_H
#define LXL_H

#include <stdio.h>        /* FILE */
#include <cbl/arena.h>    /* arena_t */


/* used in ctx.h */

typedef struct lxl_node_t lxl_node_t;
typedef struct lxl_t lxl_t;


#include "../src/alist.h"
#include "ctx.h"
#include "lex.h"


/* node kinds in lexical list */
enum {
    LXL_KHEAD,     /* dummy head */
    LXL_KSTART,    /* start of expansion; emit space if necessary */
    LXL_KTOK,      /* ordinary token */
    LXL_KTOKI,     /* ignored token */
    LXL_KEND,      /* end of expansion; emit space if necessary */
    LXL_KEOL       /* end of list */
};

/* lexical node */
struct lxl_node_t {
    unsigned char kind;      /* node kind */
    unsigned char strgno;    /* arena slot # */
    union {
        struct {
            lex_t *tok;          /* token */
            unsigned blue: 1;    /* true if painted blue */
        } t;                     /* for LXL_KTOK */
        struct {
            const char *n;            /* macro name being expanded */
            const lex_pos_t *ppos;    /* locus of macro */
        } e;                          /* for LXL_KSTART and LXL_KEND */
    } u;
    struct lxl_node_t *next;    /* next node */
};

/* lexical list */
struct lxl_t {
    lxl_node_t *head,    /* head node */
               *tail;    /* tail node */
};


lxl_t *lxl_new(arena_t *);
lxl_node_t *lxl_append(lxl_t *, int kind, ...);
lxl_t *lxl_copy(const lxl_t *);
lxl_t *lxl_tolxl(const alist_t *);
lxl_node_t *lxl_insert(lxl_t *, lxl_node_t *, lxl_t *);
lex_t *lxl_next(void);
void lxl_print(const lxl_t *, const lxl_node_t *, FILE *);


#endif    /* LXL_H */

/* end of lxl.h */
