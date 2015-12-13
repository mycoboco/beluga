/*
 *  bit-vector (cdsl)
 */

#ifndef BITV_H
#define BITV_H

#include <stddef.h>    /* size_t */


/* bit-vector */
typedef struct bitv_t bitv_t;


bitv_t *bitv_new(size_t);
void bitv_free(bitv_t **);
size_t bitv_length(const bitv_t *);
size_t bitv_count(const bitv_t *);
int bitv_get(const bitv_t *, size_t);
int bitv_put(bitv_t *, size_t, int);
void bitv_set(bitv_t *, size_t, size_t);
void bitv_clear(bitv_t *, size_t, size_t);
void bitv_not(bitv_t *, size_t, size_t);
void bitv_setv(bitv_t *, unsigned char *, size_t);
void bitv_map(bitv_t *, void (size_t, int, void *), void *);
int bitv_eq(const bitv_t *, const bitv_t *);
int bitv_leq(const bitv_t *, const bitv_t *);
int bitv_lt(const bitv_t *, const bitv_t *);
bitv_t *bitv_union(const bitv_t *, const bitv_t *);
bitv_t *bitv_inter(const bitv_t *, const bitv_t *);
bitv_t *bitv_minus(const bitv_t *, const bitv_t *);
bitv_t *bitv_diff(const bitv_t *, const bitv_t *);


#endif    /* BITV_H */

/* end of bitv.h */
