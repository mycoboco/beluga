/*
 *  storage
 */

#include <cbl/arena.h>     /* arena_t, ARENA_NEW, ARENA_DISPOSE */
#include <cbl/assert.h>    /* assert */

#include "common.h"
#include "strg.h"


arena_t *strg_perm,    /* permanent arena */
        *strg_func,    /* function arena */
        *strg_stmt,    /* statement arena */
        *strg_line;    /* line arena for preprocessing */


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
}


/*
 *  cleans up arenas and deallocates encoding buffers
 */
void (strg_close)(void)
{
    assert(strg_stmt);
    assert(strg_func);
    assert(strg_perm);
    assert(strg_line);

    ARENA_DISPOSE(&strg_stmt);
    ARENA_DISPOSE(&strg_func);
    ARENA_DISPOSE(&strg_perm);
    ARENA_DISPOSE(&strg_line);
}

/* end of strg.c */
