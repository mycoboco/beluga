/*
 *  storage for preprocessing
 */

#ifndef STRG_H
#define STRG_H

#include <cbl/arena.h>    /* arena_t */


extern arena_t *strg_perm,    /* permanent arena */
               *strg_line,    /* line arena */
               **strg_tok;    /* points to token arena; strg_perm or strg_line */
extern int strg_no;    /* current line arena slot # */


void strg_init(void);
void strg_freel(int);
void strg_close(void);


#endif    /* STRG_H */
