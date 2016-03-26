/*
 *  expression for preprocessing
 */

#ifndef EXPR_H
#define EXPR_H

#include "lex.h"

/* size of pp arithmetic types in byte;
   that is, sizeof(long) in C90 and sizeof(intmax_t) afterward */
#ifndef PPINT_BYTE
#define PPINT_BYTE 4
#endif    /* PPINT_BYTE */


/* ASSUMPTION: the host has types to contain pp arithmetic types for the target;
   ASSUMPTION: sint_t is signed counterpart of uint_t and vice versa */
typedef long sint_t;             /* signed type for pp arithmetic */
typedef unsigned long uint_t;    /* unsigned type for pp arithmetic */

/* type codes */
enum {
    EXPR_TS,    /* sint_t */
    EXPR_TU     /* uint_t */
};

/* expression value */
typedef struct expr_t {
    unsigned char type;    /* type code */
    union {
        sint_t s;    /* for signed */
        uint_t u;    /* for unsigned */
    } u;             /* value */
    unsigned posf;    /* 1: pos set for &&, 2: for comparisons */
    lex_pos_t pos;    /* locus for unparenthesized subexpr */
} expr_t;


expr_t *expr_start(lex_t **, const char *);


#endif    /* EXPR_H */

/* end of expr.h */
