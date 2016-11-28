/*
 *  storage
 */

#ifndef STRG_H
#define STRG_H

#include <cbl/arena.h>    /* arena_t */

#include "common.h"


extern arena_t *strg_perm,    /* permanent arena */
               *strg_func,    /* function arena */
               *strg_stmt,    /* statement arena */
               *strg_line;    /* line arena */

/* moved out of clx.c to manage (de)allocation */
extern char *strg_sbuf;    /* buffer for recognizing strings in clx.c */
extern sz_t strg_slen;     /* length of buffer */


void strg_init(void);
void strg_get(void);
void strg_free(arena_t *);
void strg_close(void);


#endif    /* STRG_H */

/* end of strg.h */
