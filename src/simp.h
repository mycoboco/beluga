/*
 *  constant-folding and optimization
 */

#ifndef SIMP_H
#define SIMP_H

#include "common.h"
#include "tree.h"
#include "ty.h"


extern int simp_needconst;    /* > 0 while a constant is necessary */


tree_t *simp_intexpr(int, sx_t *, int, ux_t, const char *, const lmap_t *);
tree_t *simp_basetree(const sym_t *, tree_t *);
tree_t *simp_tree(int, ty_t *, tree_t *, tree_t *, tree_pos_t *);
tree_t *simp_cvtree(int, ty_t *, ty_t *, tree_t *);


#endif    /* SIMP_H */

/* end of simp.h */
