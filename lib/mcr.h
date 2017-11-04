/*
 *  macro for preprocessing
 */

#ifndef MCR_H
#define MCR_H

#include <string.h>    /* strcmp */

#include "err.h"
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

 /* checks if __VA_ARGS__ */
#define MCR_ISVAARGS(s)    \
    ((s)[0] == '_' && (s)[1] == '_' && (s)[2] == 'V' && strcmp((s)+3, "A_ARGS__") == 0)

/* issues diagnostics when __VA_ARGS__ encountered */
#define MCR_IDVAARGS(s, t)                                          \
    do {                                                            \
        if (MCR_ISVAARGS(s) && !(t)->f.vaarg)                       \
            err_dpos((t)->pos, ERR_PP_VAARGS), (t)->f.vaarg = 1;    \
    } while(0)


#endif    /* MCR_H */

/* end of mcr.h */
