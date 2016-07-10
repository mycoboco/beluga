/*
 *  input handling
 */

#ifndef IN_H
#define IN_H

#include <stdio.h>    /* FILE */

#include "common.h"


extern long in_py;                   /* physical line # of current file */
extern const char *in_line;          /* beginning of current line */
extern const char *in_cp;            /* current character */
extern const char *in_limit;         /* end of current input buffer */
extern void (*in_nextline)(void);    /* function to read next input line */


void in_init(FILE *, const char *);
void in_close(void);
long in_getwx(const char *, const char *);


#define IN_MAXTOKEN 32    /* max length of common tokens */


#endif    /* IN_H */

/* end of in.h */
