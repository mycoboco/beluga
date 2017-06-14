/*
 *  hash (cdsl)
 */

#ifndef HASH_H
#define HASH_H

#include <stddef.h>    /* size_t */


const char *hash_string(const char *);
const char *hash_int(long);
const char *hash_new(const char *, size_t);
void hash_vload(const char *, ...);
void hash_aload(const char *[]);
void hash_free(const char *);
void hash_reset(void);
size_t hash_length(const char *);


#endif    /* HASH_H */

/* end of hash.h */
