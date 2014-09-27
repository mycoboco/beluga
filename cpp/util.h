/*
 *  utilities for preprocessing
 */

#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>    /* size_t */


size_t snlen(const char *, size_t);
lex_t *skip(lex_t *, lex_t *(*)(void));
char *snbuf(size_t, int);


#endif    /* UTIL_H */

/* end of util.h */
