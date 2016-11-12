/*
 *  utilities
 */

#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>    /* size_t */


const char *rpath(const char *path);
unsigned long utf8to32(const char **);
int wcwidth(unsigned long);
size_t snlen(const char *, size_t);
char *snbuf(size_t, int);


#endif    /* UTIL_H */

/* end of util.h */
