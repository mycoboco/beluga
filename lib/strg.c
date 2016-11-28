/*
 *  storage
 */

#include <cbl/arena.h>     /* arena_t, ARENA_NEW, ARENA_DISPOSE */
#include <cbl/assert.h>    /* assert */
#include <cbl/memory.h>    /* MEM_ALLOC, MEM_FREE */

#include "common.h"
#include "strg.h"


arena_t *strg_perm,    /* permanent arena */
        *strg_func,    /* function arena */
        *strg_stmt,    /* statement arena */
        *strg_line;    /* line arena */

/* moved out of clx.c to manage (de)allocation */
char *strg_sbuf;    /* buffer for recognizing strings in clx.c */
sz_t strg_slen;     /* length of buffer */


/* arena pool */
static struct flist {
    arena_t *strg;
    struct flist *next;
} *flist;


/*
 *  initializes arenas and encoding buffers
 */
void (strg_init)(void)
{
    assert(!strg_perm);
    assert(!strg_func);
    assert(!strg_stmt);
    assert(!strg_line);

    strg_perm = ARENA_NEW();
    strg_func = ARENA_NEW();
    strg_stmt = ARENA_NEW();
    strg_line = ARENA_NEW();

    strg_sbuf = MEM_ALLOC(strg_slen = BUFUNIT);
}


/*
 *  allocates a new arena to the line arena
 */
void (strg_get)(void)
{
    if (flist) {
        strg_line = flist->strg;
        flist = flist->next;
    } else
        strg_line = ARENA_NEW();
}


/*
 *  adds a line arena to the free list
 */
void (strg_free)(arena_t *s)
{
    struct flist *p;

    assert(s);

    ARENA_FREE(s);
    p = ARENA_ALLOC(strg_perm, sizeof(*p));
    p->strg = s;
    p->next = flist;
    flist = p;
}


/*
 *  cleans up arenas
 */
void (strg_close)(void)
{
    struct flist *p;

    assert(strg_stmt);
    assert(strg_func);
    assert(strg_perm);
    assert(strg_line);

    for (p = flist; p; p = p->next)
        ARENA_DISPOSE(&p->strg);

    ARENA_DISPOSE(&strg_stmt);
    ARENA_DISPOSE(&strg_func);
    ARENA_DISPOSE(&strg_perm);
    ARENA_DISPOSE(&strg_line);

    MEM_FREE(strg_sbuf);
}

/* end of strg.c */
