/*
 *  arena linked-list
 */

#ifndef ALIST_H
#define ALIST_H

#include <cbl/arena.h>    /* arena_t */

#include "common.h"


/* list node */
typedef struct alist_t {
    void *data;              /* data */
    struct alist_t *next;    /* next node */
} alist_t;


alist_t *alist_append(alist_t *, void *, arena_t *);
sz_t alist_length(const alist_t *);
void **alist_toarray(const alist_t *, arena_t *);


/* traverses every node in list;
   n used internally, the current node is referred to by p */
#define ALIST_FOREACH(n, p, l)                                                            \
            for ((n)=0, (void)(!(l) || ((n)=alist_length(l), (p)=(l)->next)); (n) > 0;    \
                 (n)--, (p)=(p)->next)


#endif    /* ALIST_H */

/* end of alist.h */
