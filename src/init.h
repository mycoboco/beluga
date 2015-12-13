/*
 *  initializer parsing
 */

#ifndef INIT_H
#define INIT_H

#include "ty.h"


/* segments */
enum {
    INIT_SEGCODE = 1,    /* code */
    INIT_SEGBSS,         /* uninitialized */
    INIT_SEGDATA,        /* initialized */
    INIT_SEGLIT          /* literals */
};


void init_skip(void);
ty_t *init_init_s(ty_t *, int);
int init_swtoseg(int);
int init_curseg(void);

#endif    /* INIT_H */

/* end of init.h */
