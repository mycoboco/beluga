/*
 *  expression parsing
 */

#ifndef EXPR_H
#define EXPR_H

#include "tree.h"


extern double expr_refinc;    /* weight for reference counter */


tree_t *expr_asgn(int, int, int);
tree_t *expr_expr(int, int, int);
tree_t *expr_expr0(int, int);


#endif    /* EXPR_H */

/* end of expr.h */
