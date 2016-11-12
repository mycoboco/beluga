/*
 *  macro for preprocessing
 */

#ifndef MCR_H
#define MCR_H

#include "lex.h"


void mcr_eadd(const char *);
void mcr_edel(const char *);
int mcr_redef(const char *);
void mcr_del(lex_t *);
lex_t *mcr_define(const lmap_t *, int);
void mcr_cmd(int, const char *);
void mcr_init(void);
int mcr_expand(lex_t *);
void mcr_free(void);


/* adds or removes command line macro definitions */
#define mcr_addcmd(a) (mcr_cmd(0, (a)))
#define mcr_delcmd(a) (mcr_cmd(1, (a)))


#endif    /* MCR_H */

/* end of mcr.h */
