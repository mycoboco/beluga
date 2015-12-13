/*
 *  storage
 */

#include <cbl/arena.h>     /* arena_t, ARENA_NEW, ARENA_DISPOSE */
#include <cbl/assert.h>    /* assert */
#include <cbl/memory.h>    /* MEM_ALLOC, MEM_FREE */

#include "common.h"
#include "lex.h"
#include "strg.h"
#ifdef HAVE_ICONV
#include "err.h"
#endif    /* HAVE_ICONV */


arena_t *strg_perm,    /* permanent arena */
        *strg_func,    /* function arena */
        *strg_stmt;    /* statement arena */


/*
 *  initializes arenas and encoding buffers
 */
void (strg_init)(void)
{
    assert(!strg_perm);
    assert(!strg_func);
    assert(!strg_stmt);

    assert(!lex_buf.s.p);
    assert(!lex_buf.b.p);
    assert(!lex_buf.t.p);
#ifdef HAVE_ICONV
    assert(!err_cvbuf);
#endif    /* HAVE_ICONV */

    strg_perm = ARENA_NEW();
    strg_func = ARENA_NEW();
    strg_stmt = ARENA_NEW();

    /* encoding conversion buffers allocated here for other modules have no chance to deallocate;
       their size maintained in each module and matched to initial size below */
    lex_buf.b.p = MEM_ALLOC(lex_buf.b.n);
    lex_buf.s.p = MEM_ALLOC(lex_buf.s.n + 1);    /* +1 for NUL; not included in length */
    lex_buf.t.p = MEM_ALLOC(lex_buf.t.n * sizeof(*lex_buf.t.p));
#ifdef HAVE_ICONV
    err_cvbuf = MEM_ALLOC(TL_LINE);
#endif    /* HAVE_ICONV */
}


/*
 *  cleans up arenas and deallocates encoding buffers
 */
void (strg_close)(void)
{
    assert(strg_stmt);
    assert(strg_func);
    assert(strg_perm);

    assert(lex_buf.s.p);
    assert(lex_buf.b.p);
    assert(lex_buf.t.p);
#ifdef HAVE_ICONV
    assert(err_cvbuf);
#endif    /* HAVE_ICONV */

    ARENA_DISPOSE(&strg_stmt);
    ARENA_DISPOSE(&strg_func);
    ARENA_DISPOSE(&strg_perm);

    MEM_FREE(lex_buf.s.p);    /* lex_bp may not point to lex_buf */
    MEM_FREE(lex_buf.b.p);
    MEM_FREE(lex_buf.t.p);
#ifdef HAVE_ICONV
    MEM_FREE(err_cvbuf);
#endif    /* HAVE_ICONV */
}

/* end of strg.c */
