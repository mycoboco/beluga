/*
 *  initializer parsing
 */

#ifndef INIT_H
#define INIT_H

#include "lmap.h"
#include "ty.h"


/* segments */
enum {
    INIT_SEGCODE = 1,    /* code */
    INIT_SEGBSS,         /* uninitialized */
    INIT_SEGDATA,        /* initialized */
    INIT_SEGLIT          /* literal */
};


void init_skip(void);
ty_t *init_init(ty_t *, int, const lmap_t *);
int init_swtoseg(int);
int init_curseg(void);

#endif    /* INIT_H */

/* end of init.h */
