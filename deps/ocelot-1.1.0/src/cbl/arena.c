/*
 *  arena (cbl)
 */

#include <stddef.h>    /* size_t, NULL */
#include <stdlib.h>    /* malloc, free */
#include <string.h>    /* memset */
#if __STDC_VERSION__ >= 199901L    /* C99 supported */
#include <stdint.h>    /* uintptr_t */
#endif    /* __STDC_VERSION__ */

#include "cbl/assert.h"    /* assert with exception support */
#include "cbl/except.h"    /* EXCEPT_RAISE, except_raise */
#include "arena.h"


/* smallest multiple of y greater than or equal to x */
#define MULTIPLE(x, y) ((((x)+(y)-1)/(y)) * (y))

/* checks if pointer aligned properly */
#define ALIGNED(p) ((uintptr_t)(p) % sizeof(union align) == 0)

/* max number of memory chunks in freelist */
#define FREE_THRESHOLD 10


#if __STDC_VERSION__ >= 199901L    /* C99 supported */
#    ifndef UINTPTR_MAX    /* C99, but uintptr_t not provided */
#    error "No integer type to contain pointers without loss of information!"
#    endif    /* UINTPTR_MAX */
#else    /* C90, uintptr_t surely not supported */
typedef unsigned long uintptr_t;
#endif    /* __STDC_VERSION__ */

/*
 *  An arena is consisted of a list of memory chunks. Each chuck has struct arena_t at its start
 *  address, and a memory area that can be used by an application follows it.
 *
 *  Three pointer members of struct arena_t  point to somewhere in the previous memory chunk. For
 *  example, suppose that there is only one memory chunk allocated so far. The head node (the node
 *  pointed to by arena below) that is remembered by an application and passed to, say,
 *  arena_free() has three pointers, each of which points to somewhere in that single allocated
 *  chunk. And its arena_t parts are all set to a null pointer. The following depicts this
 *  situation:
 *
 *     +---+ <----\       +---+ <------ arena
 *     | 0 |       \------|   | prev
 *     +---+              +---+
 *     | 0 |         /----|   | avail
 *     +---+        /     +---+
 *     | 0 |       /  /---|   | limit
 *     +---+      /  /    +---+
 *     | . |     /  /
 *     | A | <--/  /
 *     | . |      /
 *     +---+ <---/
 *
 *  After one more chunk has been allocated, arena points to it, and the arena_t parts of the new
 *  chunk point to the previous chunk marked A as shown below:
 *
 *     +---+ <----\       +---+ <----\       +---+ <------ arena
 *     | 0 |       \------|   |       \------|   | prev
 *     +---+              +---+              +---+
 *     | 0 |          /---|   |         /----|   | avail
 *     +---+         /    +---+        /     +---+
 *     | 0 |        / /---|   |       /  /---|   | limit
 *     +---+       / /    +---+      /  /    +---+
 *     | . |      / /     | . |     /  /
 *     | A |     / /      | B | <--/  /
 *     | . | <--/ /       | . |      /
 *     +---+ <---/        +---+ <---/
 *
 *  This can be thought as pushing a new memory chunk through the head node.
 */
struct arena_t {
    struct arena_t *prev;    /* previously allocated chunk */
    char *avail;             /* start of available area in previous chunk */
    char *limit;             /* end of previous chunk */
};

/*
 *  union align tries to automatically determine the maximum alignment requirement imposed by an
 *  implementation; if you know the exact restriction, define MEM_MAXALIGN properly - a compiler
 *  option like -D should be a proper place for it. If the guess is wrong, the memory library is
 *  not guaranteed to work properly, or more severely programs may crash.
 *
 *  Even if this library employs the prefix arena_ or ARENA_, the prefix of the memory library
 *  MEM_ is used below because union align is first introduced there.
 */
union align {
#ifdef MEM_MAXALIGN
    char pad[MEM_MAXALIGN];
#else
    int i;
    long l;
    long *lp;
    void *p;
    void (*fp)(void);
    float f;
    double d;
    long double ld;
#endif
};

/*
 *  As shown in the explanation of arena_t, the memory area that is to be used by an application is
 *  attached after the arena_t part. Because its starting address should be properly aligned as
 *  those returned by malloc() are, it might be necessary to put some padding between the arena_t
 *  part and the user memory area. union header do this job using union align. It ensures that
 *  there is enough space for arena_t and the starting address of the following user area is
 *  properly aligned; there should be some padding after the member a if necessary in order to make
 *  the second element properly aligned in an array of the union header type.
 */
union header {
    arena_t b;
    union align a;
};


/* exception for arena creation failure */
const except_t arena_exceptfailNew = { "Arena creation failed" };

/* exception for memory allocation failure */
const except_t arena_exceptfailAlloc = { "Arena allocation failed" };


/*
 *  freelist is a list of free memory chunks. When arena_free() called, before it really releases
 *  the storage with free(), it sets aside that chunk into freelist. This reduces necessity of
 *  calling malloc(). On the other hand, if freelist maintains too many instances of free chunks,
 *  invocations for allocation using other memory allocator (for example, mem_alloc() from the
 *  memory library) would fail. That is why arena_free() keeps no more than FREE_THRESHOLD chunks
 *  in freelist.
 *
 *  Differently from memory chunks described by arena_t, chunks in the free list have their limit
 *  member point to their own limits; see arena_t for comparison.
 */
static arena_t *freelist;

/*
 *  freenum is incresed when a memory chunk is pushed to freelist by arena_free() and decresed when
 *  it is pushed back to the arena_t list by arena_alloc().
 */
static int freenum;


/*
 *  creates a new arena
 */
arena_t *(arena_new)(void)
{
    arena_t *arena;

    arena = malloc(sizeof(*arena));    /* use malloc() to separate arena library from a specific
                                          memory allocator like memory library */
    if (!arena)
        EXCEPT_RAISE(arena_exceptfailNew);

    arena->prev = NULL;
    arena->limit = arena->avail = NULL;

    return arena;
}


/*
 *  disposes an arena
 */
void (arena_dispose)(arena_t **parena)
{
    assert(parena);
    assert(*parena);

    arena_free(*parena);
    free(*parena);
    *parena = NULL;
}


/*
 *  allocates storage associated with an arena
 *
 *  There are three cases where arena_alloc() successfully returns storage:
 *  - if the first chunk in the arena_t list has enough space for the request, it is returned;
 *  - otherwise, the first chunk in the free list (if any) is pushed back to the arena_t list and
 *    go to the first step;
 *  - if there is nothing in the free list, new storage is allocated by malloc() and pushed to the
 *    arena_t list and go to the first step.
 *
 *  Note that the second case is not able to make a sure find while the third is, which is why
 *  arena_alloc() contains a loop.
 *
 *  The original code in the book does not check if the limit or avail member of arena is a null
 *  pointer; by definition, operations like comparing two pointers, subtracting a pointer from
 *  another and adding an integer to a pointer result in undefined behavior if they are involved
 *  with a null pointer, thus fixed here.
 */
#if __STDC_VERSION__ >= 199901L    /* C99 version */
void *(arena_alloc)(arena_t *arena, size_t n, const char *file, const char *func, int line)
#else    /* C90 version */
void *(arena_alloc)(arena_t *arena, size_t n, const char *file, int line)
#endif    /* __STDC_VERSION__ */
{
    assert(arena);
    assert(n > 0);

    n = MULTIPLE(n, sizeof(union align));

    assert(arena->limit >= arena->avail);
    /* first request or requested size > left size of first chunk */
    while (!arena->limit || n > arena->limit - arena->avail) {
        arena_t *p;
        char *limit;

        if ((p = freelist) != NULL) {    /* free chunks exist in freelist */
            freelist = freelist->prev;
            freenum--;    /* chunk to be pushed back to arena_t list, so decresed */
            limit = p->limit;
        } else {    /* allocation needed */
            size_t m = sizeof(union header) + n + 10*1024;    /* enough to save arena_t + requested
                                                                 size + extra (10Kb) */
            p = malloc(m);
            if (!p) {
                if (!file)
                    EXCEPT_RAISE(arena_exceptfailAlloc);
                else
#if __STDC_VERSION__ >= 199901L    /* C99 version */
                    except_raise(&arena_exceptfailAlloc, file, func, line);
#else    /* C90 version */
                    except_raise(&arena_exceptfailAlloc, file, line);
#endif    /* __STDC_VERSION__ */
            }
            assert(ALIGNED(p));    /* checks if guess at alignment restriction holds - if fails,
                                      define MEM_MAXALIGN properly */
            limit = (char *)p + m;
        }
        *p = *arena;    /* copies previous arena info to newly allocated chunk */
        /* makes head point to newly pushed chunk */
        arena->avail = (char *)((union header *)p + 1);
        arena->limit = limit;
        arena->prev  = p;
    }
    /* chunk having free space with enough size found */
    arena->avail += n;

    return arena->avail - n;
}


/*
 *  allocates zero-filled storage associated with an arena
 *
 *  TODO:
 *    - the C standard requires calloc() return a null pointer if it cannot allocates storage of
 *      the size c * n in byte, which allows no overflow in computing the multiplication.
 *      Overflow checking is necessary to mimic the behavior of calloc().
 */
#if __STDC_VERSION__ >= 199901L    /* C99 version */
void *(arena_calloc)(arena_t *arena, size_t c, size_t n, const char *file, const char *func,
                     int line)
#else    /* C90 version */
void *(arena_calloc)(arena_t *arena, size_t c, size_t n, const char *file, int line)
#endif    /* __STDC_VERSION__ */
{
    void *p;

    assert(c > 0);

#if __STDC_VERSION__ >= 199901L    /* C99 version */
    p = arena_alloc(arena, c*n, file, func, line);
#else    /* C90 version */
    p = arena_alloc(arena, c*n, file, line);
#endif    /* __STDC_VERSION__ */
    memset(p, '\0', c*n);

    return p;
}


/*
 *  deallocates all storages belonging to an arena
 *
 *  arena_free() does its job by by popping memory chunks belonging to an arena until it gets
 *  empty. Those popped chunks are released by free() if there are already FREE_THRESHOLD chunks in
 *  freelist, or pushed to freelist otherwise.
 */
void (arena_free)(arena_t *arena)
{
    assert(arena);

    while (arena->prev) {
        arena_t tmp = *arena->prev;
        if (freenum < FREE_THRESHOLD) {    /* need to set aside to freelist */
            arena->prev->prev = freelist;    /* prev of to-be-freed = existing freelist */
            freelist = arena->prev;          /* freelist = to-be-freed */
            freenum++;
            freelist->limit = arena->limit;    /* in the free list, each chunk has the limit member
                                                  point to its own limit; see arena_t for
                                                  comparison */
        } else    /* freelist is full; deallocate */
            free(arena->prev);
        *arena = tmp;
    }

    /* all are freed here */
    assert(!arena->limit);
    assert(!arena->avail);
}

/* end of arena.c */
