/*
 *  expression for preprocessing
 */

#ifndef EXPR_H
#define EXPR_H

#include "common.h"
#include "lex.h"
#include "lmap.h"

/* size of pp arithmetic types in byte on the target;
   that is, sizeof(long) in C90 and sizeof(intmax_t) afterward */
#ifndef PPINT_BYTE
#ifdef SUPPORT_LL
#define PPINT_BYTE 8
#else    /* !SUPPORT_LL */
#define PPINT_BYTE 4
#endif    /* SUPPORT_LL */
#endif    /* PPINT_BYTE */


/* type code */
enum {
    EXPR_TS,    /* signed */
    EXPR_TU     /* unsigned */
};

/* expression value */
typedef struct expr_t {
    unsigned char type;    /* type code */
    union {
        sx_t s;    /* for signed */
        ux_t u;    /* for unsigned */
    } u;
    int posf;              /* 1: pos for &&, 2: for comparison */
    const lmap_t *spos;    /* start locus of subexpression */
    const lmap_t *epos;    /* end locus of subexpression */
} expr_t;


expr_t *expr_start(lex_t **, const char *);


#endif    /* EXPR_H */

/* end of expr.h */
