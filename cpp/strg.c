/*
 *  storage for preprocessor
 */

#include <cbl/arena.h>     /* arena_t, ARENA_NEW, ARENA_FREE, ARENA_DISPOSE */
#include <cbl/assert.h>    /* assert */
#ifdef HAVE_ICONV
#include <cbl/memory.h>    /* MEM_ALLOC, MEM_FREE */
#endif    /* HAVE_ICONV */

#include "../src/common.h"
#include "../src/err.h"
#include "../src/in.h"


arena_t *strg_perm,                 /* permanent arena */
        *strg_line,                 /* line arena; one of line[] */
        **strg_tok = &strg_line;    /* points to token arena; strg_perm or strg_line */
int strg_no;    /* current line arena slot # */


static arena_t *line[8];    /* line arena slots; size must be power of 2 */


/*
 *  initializes arenas and encoding buffers
 */
void (strg_init)(void)
{
    int i;

    assert(!strg_perm);
    assert(!strg_line);
#ifdef HAVE_ICONV
    assert(!err_cvbuf);
#endif    /* HAVE_ICONV */

    strg_perm = ARENA_NEW();
    for (i = 0; i < NELEM(line); i++)
        line[i] = ARENA_NEW();
    strg_line = line[0];
#ifdef HAVE_ICONV
    err_cvbuf = MEM_ALLOC(TL_LINE);
#endif    /* HAVE_ICONV */
}


/*
 *  frees one of the previous line arenas
 */
void (strg_freel)(int strgno)
{
    strg_no = (strgno+1) & (NELEM(line)-1);
    ARENA_FREE(line[strg_no]);

    strg_line = line[strg_no];
    if (strg_no == 0) {
        in_exitfunc();
        in_enterfunc();
    }
}


/*
 *  cleans up arenas and deallocates encoding buffers
 */
void (strg_close)(void)
{
    int i;

    assert(strg_perm);
    assert(strg_line);
#ifdef HAVE_ICONV
    assert(err_cvbuf);
#endif    /* HAVE_ICONV */

    for (i = 0; i < NELEM(line); i++)
        ARENA_DISPOSE(&line[i]);
    ARENA_DISPOSE(&strg_perm);
#ifdef HAVE_ICONV
    MEM_FREE(err_cvbuf);
#endif    /* HAVE_ICONV */
}

/* end of strg.c */
