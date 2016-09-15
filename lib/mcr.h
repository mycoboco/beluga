/*
 *  macro for preprocessing
 */

#ifndef MCR_H
#define MCR_H

#include "lex.h"


void mcr_eadd(const char *);
void mcr_edel(const char *);
int mcr_redef(const char *);
void mcr_del(const char *, const lmap_t *);
lex_t *mcr_define(int cmd);
void mcr_init(void);
int mcr_expand(lex_t *);
void mcr_free(void);


#endif    /* MCR_H */

/* end of mcr.h */
