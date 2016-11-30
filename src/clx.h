/*
 *  compiler lexical analyzer
 */

#ifndef CLX_H
#define CLX_H

#include "lex.h"
#include "lmap.h"
#include "sym.h"


void clx_init(void);
ux_t clx_ccon(lex_t *, int *);
int clx_next(void);
void clx_close(void);


extern int clx_tc;                /* token code of current token */
extern const char *clx_tok;       /* token spelling of current token */
extern sym_t *clx_sym;            /* symbol table entry for current token */
extern const lmap_t *clx_cpos;    /* locus of current token */
extern const lmap_t *clx_ppos;    /* locus of previous token */

extern const char * const clx_name[];    /* token names */
extern const char clx_kind[];            /* token kinds */


#endif    /* CLX_H */

/* end of clx.h */
