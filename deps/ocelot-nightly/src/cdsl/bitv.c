/*
 *  bit-vector (cdsl)
 */

#include <stddef.h>    /* size_t, NULL */
#include <string.h>    /* memcpy */

#include "cbl/memory.h"    /* MEM_ALLOC, MEM_NEW, MEM_FREE */
#include "cbl/assert.h"    /* assert with exception support */
#include "bitv.h"


#define BPW (8 * sizeof(unsigned long))    /* number of bits per word */

#define nword(len) (((len)+BPW-1) / BPW)    /* number of words for bit-vector of length len */
#define nbyte(len) (((len)+8-1) / 8)        /* number of bytes for bit-vector of length len */

#define BIT(set, n) (((set)->byte[(n)/8] >> ((n)%8)) & 1)    /* extracts bit from bit-vector */

/* body for work on range */
#define range(op, fcmp, cmp)                                   \
    do {                                                       \
        assert(set);                                           \
        assert(l <= h);                                        \
        assert(h < set->length);                               \
        if (l/8 < h/8) {                                       \
            set->byte[l/8] op##= cmp msb[l%8];                 \
            {                                                  \
                size_t i;                                      \
                for (i = l/8 + 1; i < h/8; i++)                \
                    set->byte[i] = fcmp 0U;                    \
                set->byte[h/8] op##= cmp lsb[l%8];             \
            }                                                  \
        } else                                                 \
            set->byte[l/8] op##= cmp (msb[l%8] & lsb[h%8]);    \
    } while(0)

/* body for set operations */
#define setop(eq, sn, tn, op)                               \
    do {                                                    \
        assert(s || t);                                     \
        if (s == t) return (eq);                            \
        else if (!s) return (sn);                           \
        else if (!t) return (tn);                           \
        else {                                              \
            size_t i;                                       \
            bitv_t *set;                                    \
            assert(s->length == t->length);                 \
            set = bitv_new(s->length);                      \
            for (i = 0; i < nword(s->length); i++)          \
                set->word[i] = s->word[i] op t->word[i];    \
            return set;                                     \
        }                                                   \
    } while(0)


/* bit masks for msbs */
static unsigned msb[] = {
    0xFF, 0xFE, 0xFC, 0xF8, 0xF0, 0xE0, 0xC0, 0x80
};

/* bit masks for lsbs */
static unsigned lsb[] = {
    0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE, 0xFF
};

/* bit mask for paddings */
static unsigned pad[] = {
    0xFF, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F
};


/*
 *  bit-vector
 *
 *  struct bitv_t contains a sequence of words to implement a bit-vector whose length in bits is
 *  held in length. For table-driven approaches, byte provides access to an individual byte in
 *  words for a bit-vector. Unused padding bits or bytes are unavoidable without bit-wise storage
 *  allocation, and they should have no effect on the result because always set to zeros.
 */
struct bitv_t {
    size_t length;          /* length in bits for bit-vector */
    unsigned char *byte;    /* byte-wise access to bit-vector words */
    unsigned long *word;    /* words to contain bit-vector */
};


/*
 *  creates a copy of a bit-vector
 */
static bitv_t *copy(const bitv_t *t)
{
    bitv_t *set;

    assert(t);

    set = bitv_new(t->length);
    if (t->length != 0)
        memcpy(set->byte, t->byte, nbyte(t->length));

    return set;
}


/*
 *  creates a new bit-vector
 */
bitv_t *(bitv_new)(size_t len) {
    bitv_t *set;

    MEM_NEW(set);
    set->word = (len > 0)? MEM_CALLOC(nword(len), sizeof(unsigned long)): NULL;
    set->byte = (void *)set->word;
    set->length = len;

    return set;
}


/*
 *  destroys a bit-vector
 */
void (bitv_free)(bitv_t **pset) {
    assert(pset);
    assert(*pset);

    MEM_FREE((*pset)->word);
    MEM_FREE(*pset);
}


/*
 *  returns the length of a bit-vector
 */
size_t (bitv_length)(const bitv_t *set) {
    assert(set);

    return set->length;
}


/*
 *  returns the number of bits set
 */
size_t (bitv_count)(const bitv_t *set)
{
    static char count[] = { 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4 };

    size_t i, c = 0;

    assert(set);

    for (i = 0; i < nbyte(set->length); i++) {
        unsigned char ch = set->byte[i];
        c += count[ch & 0x0F] + count[ch >> 4];
    }

    return c;
}


/*
 *  gets a bit in a bit-vector
 */
int (bitv_get)(const bitv_t *set, size_t n)
{
    assert(set);
    assert(n < set->length);

    return BIT(set, n);
}


/*
 *  changes the value of a bit in a bit-vector
 */
int (bitv_put)(bitv_t *set, size_t n, int bit)
{
    int prev;

    assert(set);
    assert((bit & 1) == bit);
    assert(n < set->length);

    prev = BIT(set, n);
    if (bit)
        set->byte[n/8] |= 1U << (n%8);
    else
        set->byte[n/8] &= ~(1U << (n%8));

    return prev;
}


/*
 *  sets bits to 1 in a bit-vector
 */
void (bitv_set)(bitv_t *set, size_t l, size_t h)
{
    range(|, ~, +);
}


/*
 *  clears bits in a bit-vector
 */
void (bitv_clear)(bitv_t *set, size_t l, size_t h)
{
    range(&, +, ~);
}


/*
 *  complements bits in a bit-vector
 */
void (bitv_not)(bitv_t *set, size_t l, size_t h)
{
    range(^, ~, +);
}


/*
 *  sets bits in a bit-vector with bit patterns
 */
void (bitv_setv)(bitv_t *set, unsigned char *v, size_t n)
{
    size_t i;

    assert(set);
    assert(v);
    assert(n > 0);
    assert(n <= nbyte(set->length));

    if (n == nbyte(set->length))
        v[n-1] &= pad[set->length % 8];
    for (i = 0; i < n; i++)
        set->byte[i] = v[i] & 0xFF;
}


/*
 *  calls a user-provided function for each bit in a bit-vector
 */
void (bitv_map)(bitv_t *set, void apply(size_t, int, void *), void *cl)
{
    size_t i;

    assert(set);

    for (i = 0; i < set->length; i++)
        apply(i, BIT(set, i), cl);
}


/*
 *  compares two bit-vectors for equality
 */
int (bitv_eq)(const bitv_t *s, const bitv_t *t)
{
    size_t i;

    assert(s);
    assert(t);
    assert(s->length == t->length);

    for (i = 0; i < nword(s->length); i++)
        if (s->word[i] != t->word[i])
            return 0;
    return 1;
}


/*
 *  compares two bit-vectors for subset
 */
int (bitv_leq)(const bitv_t *s, const bitv_t *t)
{
    size_t i;

    assert(s);
    assert(t);
    assert(s->length == t->length);

    for (i = 0; i < nword(s->length); i++)
        if ((s->word[i] & ~t->word[i]) != 0)
            return 0;
    return 1;
}


/*
 *  compares two bit-vectors for proper subset
 */
int (bitv_lt)(const bitv_t *s, const bitv_t *t)
{
    size_t i;
    int lt = 0;

    assert(s);
    assert(t);
    assert(s->length == t->length);

    for (i = 0; i < nword(s->length); i++)
        if ((s->word[i] & ~t->word[i]) != 0)
            return 0;
        else if (s->word[i] != t->word[i])
            lt = 1;
    return lt;
}


/*
 *  returns a union of two bit-vectors
 */
bitv_t *(bitv_union)(const bitv_t *t, const bitv_t *s)
{
    setop(copy(t), copy(t), copy(s), |);
}


/*
 *  returns an intersection of two bit-vectors
 */
bitv_t *(bitv_inter)(const bitv_t *t, const bitv_t *s)
{
    setop(copy(t), bitv_new(t->length), bitv_new(s->length), &);
}


/*
 *  returns a difference of two bit-vectors
 */
bitv_t *(bitv_minus)(const bitv_t *t, const bitv_t *s)
{
    setop(bitv_new(s->length), bitv_new(t->length), copy(s), & ~);
}


/*
 *  returns a symmetric difference of two bit-vectors
 */
bitv_t *(bitv_diff)(const bitv_t *t, const bitv_t *s)
{
    setop(bitv_new(s->length), copy(t), copy(s), ^);
}

/* end of bitv.c */
