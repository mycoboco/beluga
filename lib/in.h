/*
 *  input handling
 */

#ifndef IN_H
#define IN_H

#include <stdio.h>    /* FILE */

#include "common.h"


extern sz_t in_py;                   /* physical line # of current file */
extern const char *in_line;          /* beginning of current line */
extern const char *in_cp;            /* current character */
extern const char *in_limit;         /* end of current input buffer */
extern void (*in_nextline)(void);    /* function to read next input line */


int in_trigraph(const char *);
sz_t in_cntchar(const char *, const char *, sz_t, const char **);
void in_init(FILE *, const char *);
void in_switch(FILE *);
void in_close(void);
sz_t in_getwx(sz_t, const char *, const char *, int *);


#define IN_MAXTOKEN 32    /* max length of common tokens */


#endif    /* IN_H */

/* end of in.h */
