/*
 *  include for preprocessing
 */

#ifndef INC_H
#define INC_H

#include <stdio.h>     /* FILE */

#include "cond.h"
#include "lex.h"
#include "lxl.h"


/* element for #include list */
typedef struct inc_t {
    FILE *fptr;                    /* file pointer */
    const char *f;                 /* in_cpos.f */
    unsigned long y;               /* in_cpos.y */
    const char *mf;                /* in_cpos.mf */
    unsigned long my;              /* in_cpos.my */
    const unsigned char *limit,    /* in_limit */
                        *line,     /* in_line */
                        *cp;       /* in_cp */
    const char *cwd;               /* current working directory */
    unsigned printed: 1;           /* true if already printed */
    cond_t *cond;                  /* context for conditional inclusion */
} inc_t;


extern inc_t **inc_list;    /* #include list */


void inc_add(const char *);
void inc_init(void);
void inc_free(void);
int inc_start(const char *, const lex_pos_t *);
void inc_push(FILE *);
FILE *inc_pop(FILE *);
int inc_isffile(void);


#endif    /* INC_H */

/* end of inc.h */
