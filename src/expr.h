/*
 *  expression parsing
 */

#ifndef EXPR_H
#define EXPR_H

#include "lmap.h"
#include "tree.h"


extern double expr_refinc;    /* weight for reference counter */


tree_t *expr_asgn(int, int, int, const lmap_t *);
tree_t *expr_expr(int, int, int, const lmap_t *);
tree_t *expr_expr0(int, int, const lmap_t *);


#endif    /* EXPR_H */

/* end of expr.h */
