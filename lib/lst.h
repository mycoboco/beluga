/*
 *  token list
 */

#ifndef LST_H
#define LST_H

#ifndef NDEBUG
#include <stdio.h>    /* FILE */
#endif    /* !NDEBUG */

#include "lex.h"

#define lst_peek() (lst_out->next)    /* looks ahead token from output list */


extern lex_t *lst_in, *lst_out;      /* input/output token list */
extern lex_t *(*lst_nexti)(void);    /* retrieves token from input list */


lex_t *lst_append(lex_t *, lex_t *);
lex_t *lst_insert(lex_t *, lex_t *, lex_t *);
void lst_flush(void);
void lst_discard(void);
lex_t *lst_next(void);
#ifndef NDEBUG
void lst_print(lex_t *, FILE *);
#endif    /* !NDEBUG */


#endif    /* LST_H */

/* end of lst.h */
