/*
 *  utilities
 */

#ifndef UTIL_H
#define UTIL_H


const char *rpath(const char *path);
unsigned long utf8to32(const char **);
int wcwidth(unsigned long);
int conv3(int);


#endif    /* UTIL_H */

/* end of util.h */
