/*
 *  conditional inclusion for preprocessing
 */

#ifndef COND_H
#define COND_H

#include "lex.h"


/* kind of conditionals */
enum {
    COND_KIF,        /* #if */
    COND_KIFDEF,     /* #ifdef */
    COND_KIFNDEF,    /* #ifndef */
};

/* element for conditional list */
typedef struct cond_t {
    unsigned short level;    /* nesting level */
    unsigned char kind;      /* kind of conditional inclusions */
    struct {
        unsigned once: 1;      /* true if inclusive section seen */
        unsigned ignore: 2;    /* > 0 while in ignoring section */
    } f;
    lex_pos_t ifpos;        /* locus for #if/#ifdef/#ifndef */
    lex_pos_t elsepos;      /* locus for #else */
    struct cond_t *prev;    /* enclosing nesting level */
} cond_t;


extern cond_t *cond_list;    /* conditional list */


void cond_push(int, const lex_pos_t *);
void cond_pop(void);
void cond_finalize(void);
const char *cond_name(int);


/* checks if in skipping section */
#define cond_ignore() (cond_list && cond_list->f.ignore)


#endif    /* COND_H */

/* end of cond.h */
