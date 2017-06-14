/*
 *  arena (cbl)
 */

#ifndef ARENA_H
#define ARENA_H

#include <stddef.h>    /* size_t */

#include "cbl/except.h"    /* except_t */


/* arena */
typedef struct arena_t arena_t;


/* exceptions for arena creation/allocation failure */
extern const except_t arena_exceptfailNew;
extern const except_t arena_exceptfailAlloc;


arena_t *arena_new(void);
#if __STDC_VERSION__ >= 199901L    /* C99 version */
void *arena_alloc(arena_t *, size_t, const char *, const char *, int);
void *arena_calloc(arena_t *, size_t, size_t, const char *, const char *, int);
#else    /* C90 version */
void *arena_alloc(arena_t *, size_t, const char *, int);
void *arena_calloc(arena_t *, size_t, size_t, const char *, int);
#endif    /* __STDC_VERSION__ */
void arena_free(arena_t *);
void arena_dispose(arena_t **);


/* macro wrappers for functions */
#define ARENA_NEW()       (arena_new())
#define ARENA_DISPOSE(pa) (arena_dispose(pa))
#if __STDC_VERSION__ >= 199901L    /* C99 version */
#define ARENA_ALLOC(a, n)     (arena_alloc((a), (n), __FILE__, __func__, __LINE__))
#define ARENA_CALLOC(a, c, n) (arena_calloc((a), (c), (n), __FILE__, __func__, __LINE__))
#else    /* C90 version */
#define ARENA_ALLOC(a, n)     (arena_alloc((a), (n), __FILE__, __LINE__))
#define ARENA_CALLOC(a, c, n) (arena_calloc((a), (c), (n), __FILE__, __LINE__))
#endif    /* __STDC_VERSION__ */
#define ARENA_FREE(a) (arena_free(a))


#endif    /* ARENA_H */

/* end of arena.h */
