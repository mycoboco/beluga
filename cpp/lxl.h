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
    LXL_KSTART = 1,                      /* start of expansion; emit space if necessary */
    LXL_KEND,                            /* end of expansion; emit space if necessary */
    LXL_KHEAD = 1 << 4,                  /* dummy head; never shared */
    LXL_KTOK  = LXL_KHEAD + (1 << 4),    /* ordinary token */
    LXL_KTOKI = LXL_KTOK +  (1 << 4),    /* ignored token */
    LXL_KEOL  = LXL_KTOKI + (1 << 4)     /* end of list; never shared */
};

/* lexical node */
struct lxl_node_t {
    unsigned char kind;      /* node kind */
    unsigned char strgno;    /* arena slot # */
    struct {
        lex_t *tok;          /* token */
        unsigned blue: 1;    /* true if painted blue */
    } t;                     /* for LXL_KTOK[I] */
    struct {
        const char *n;            /* macro name being expanded */
        const lex_pos_t *ppos;    /* locus of macro */
    } *e;                         /* for LXL_KSTART and LXL_KEND */
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


/* handles sharing slots */
#define lxl_pre(n)      ((n)->kind & 0x0f)
#define lxl_post(n)     ((n)->kind & 0xf0)
#define lxl_isshared(n) (lxl_pre(n) && lxl_post(n))
#define lxl_kind(n)     ((lxl_isshared(n))? lxl_pre(n): (n)->kind)

#define LXL_IGNORE(n)   ((n)->kind = lxl_pre(n) | LXL_KTOKI)    /* ignores token nodes */


#endif    /* LXL_H */

/* end of lxl.h */
