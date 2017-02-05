/*
 *  expression tree
 */

#ifndef ENODE_H
#define ENODE_H

#include "lmap.h"
#include "tree.h"
#include "ty.h"


/* flag values for enode_cast() */
enum {
    ENODE_FECAST  = 1 << 0,    /* explicit cast */
    ENODE_FCHKOVF = 1 << 1     /* checks overflow */
};


tree_t *enode_value(tree_t *);
tree_t *enode_cond(tree_t *);
tree_t *enode_pointer(tree_t *);
int enode_isnpc(tree_t *);
tree_t *enode_cast(tree_t *, ty_t *, int, const lmap_t *);

ty_t *enode_tcasgnty(ty_t *, tree_t *, const lmap_t *, tree_t *);
ty_t *enode_tcasgn(tree_t *, tree_t *, tree_pos_t *);
ty_t *enode_tccond(tree_t *, tree_t *);
ty_t *enode_tcand(tree_t *, tree_t *);
ty_t *enode_tcbit(tree_t *, tree_t *);
ty_t *enode_tceq(tree_t *, tree_t *);
ty_t *enode_tccmp(tree_t *, tree_t *);
ty_t *enode_tcsh(tree_t *, tree_t *);
ty_t *enode_tcadd(tree_t *, tree_t *);
ty_t *enode_tcsub(tree_t *, tree_t *);
ty_t *enode_tcmul(tree_t *, tree_t *);
ty_t *enode_tcposneg(tree_t *);
ty_t *enode_tcbcom(tree_t *);
ty_t *enode_tcnot(tree_t *);
ty_t *enode_tcindir(tree_t *, tree_pos_t *);
ty_t *enode_tcaddr(tree_t *);

tree_t *enode_chkcond(int, tree_t *, const char *);
tree_t *enode_tyerr(int, tree_t *, tree_t *, tree_pos_t *);


#endif    /* ENODE_H */

/* end of enode.h */
