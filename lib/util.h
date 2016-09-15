/*
 *  utilities
 */

#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>    /* size_t */

/* skip spaces */
#define SKIPSP(t) while ((t)->id == LEX_SPACE) (t) = lst_nexti()
#define NEXTSP(t) do { (t) = lst_nexti(); } while((t)->id == LEX_SPACE)

/* skips to newline */
#define SKIPNL(t) while ((t)->id != LEX_NEWLINE && (t)->id != LEX_EOI) (t) = lst_nexti()


const char *rpath(const char *path);
unsigned long utf8to32(const char **);
int wcwidth(unsigned long);
size_t snlen(const char *, size_t);
char *snbuf(size_t, int);


#endif    /* UTIL_H */

/* end of util.h */
