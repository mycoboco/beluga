/*
 *  storage for preprocessing
 */

#ifndef STRG_H
#define STRG_H

#include <cbl/arena.h>    /* arena_t */


extern arena_t *strg_perm,
               *strg_inc,
               *strg_ctx,
               *strg_line,
               **strg_tok;
extern int strg_no;


void strg_init(void);
void strg_freel(int);
void strg_close(void);


#endif    /* STRG_H */
