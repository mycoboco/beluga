/*
 *  storage
 */

#ifndef STRG_H
#define STRG_H

#include <limits.h>       /* CHAR_BIT */
#include <cbl/arena.h>    /* arena_t */

#include "common.h"

/* buffer length for decimal integer without sign */
#define STRG_BUFN ((sizeof(ux_t)*CHAR_BIT + 2) / 3)


extern arena_t *strg_perm,    /* permanent arena */
               *strg_func,    /* function arena */
               *strg_stmt,    /* statement arena */
               *strg_line;    /* line arena */

/* moved out of clx.c to manage (de)allocation */
extern char *strg_sbuf;    /* buffer for recognizing strings in clx.c */
extern sz_t strg_slen;     /* length of buffer */

/* s/ux_t stringization buffer */
extern char strg_nbuf[2 + 1 + STRG_BUFN + 1];


void strg_init(void);
void strg_get(void);
void strg_free(arena_t *);
void strg_close(void);


#endif    /* STRG_H */

/* end of strg.h */
