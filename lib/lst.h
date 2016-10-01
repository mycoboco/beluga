/*
 *  token list
 */

#ifndef LST_H
#define LST_H

#include <cbl/arena.h>    /* arena_t */
#ifndef NDEBUG
#include <stdio.h>    /* FILE */
#endif    /* !NDEBUG */

#include "lex.h"


extern lex_t *(*lst_nexti)(void);    /* retrieves token from input list */
extern lex_t *(*lst_peeki)(void);    /* looks ahead next token */


void lst_push(lex_t *);
lex_t *lst_pop(void);
lex_t *lst_append(lex_t *, lex_t *);
void lst_insert(lex_t *);
void lst_flush(int, int);
void lst_discard(int, int);
lex_t *lst_next(void);
lex_t *lst_copy(const lex_t *, int, arena_t *);
lex_t *lst_copyl(const lex_t *, int, arena_t *);
int lst_length(const lex_t *);
lex_t **lst_toarray(lex_t *, arena_t *);
lex_t *lst_run(const char *, const lmap_t *);
#ifndef NDEBUG
void lst_print(lex_t *, FILE *);
#endif    /* !NDEBUG */


#endif    /* LST_H */

/* end of lst.h */
