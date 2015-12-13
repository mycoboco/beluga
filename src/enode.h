/*
 *  expression tree
 */

#ifndef ENODE_H
#define ENODE_H

#include "tree.h"
#include "ty.h"


/* flag values for enode_cast_s() */
enum {
    ENODE_FECAST  = 1 << 0,    /* explicit cast */
    ENODE_FCHKOVF = 1 << 1     /* checks overflow */
};


tree_t *enode_value_s(tree_t *);
tree_t *enode_cond_s(tree_t *);
tree_t *enode_pointer_s(tree_t *);
ty_t *enode_binary(ty_t *, ty_t *);
int enode_isnpc_s(tree_t *);
tree_t *enode_cast_s(tree_t *, ty_t *, int);

ty_t *enode_tcasgnty_s(ty_t *, tree_t *);
ty_t *enode_tcasgn_s(tree_t *, tree_t *);
ty_t *enode_tccond_s(tree_t *, tree_t *);
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
ty_t *enode_tcindir_s(tree_t *, int *);
ty_t *enode_tcaddr(tree_t *);

tree_t *enode_chkcond(int, tree_t *, const char *);
tree_t *enode_setexperr(tree_t *);
tree_t *enode_typeerr_s(int, tree_t *, tree_t *);


/* merges expression error flags for p */
#define ENODE_MRGEXPERR(i, p)  ((i | 0)? enode_setexperr(p): (p))
#define ENODE_MRGEXPERRT(t, p) (((t)->f.experr)? enode_setexperr(p): (p))


#endif    /* ENODE_H */

/* end of enode.h */
