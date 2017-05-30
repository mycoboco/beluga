/*
 *  include for preprocessing
 */

#ifndef INC_H
#define INC_H

#include <stdio.h>    /* FILE */

#include "common.h"
#include "cond.h"
#include "lmap.h"


/* #include chain element */
typedef struct inc_t {
    FILE *fptr;             /* file pointer */
    int bs;                 /* # of escaped newlines */
    cond_t *cond;           /* context for conditional inclusion */
    int mgstate;            /* mg_state */
    const char *mgname;     /* mg_name */
} inc_t;


extern inc_t **inc_chain;    /* #include chain */


void inc_add(const char *, int);
void inc_init(void);
void inc_free(void);
int inc_start(const char *, const lmap_t *);
void inc_push(FILE *, int);
FILE *inc_pop(FILE *, sz_t *);
int inc_isffile(void);


#endif    /* INC_H */

/* end of inc.h */
