/*
 *  set (cdsl)
 */

#include <limits.h>    /* INT_MAX */
#include <stddef.h>    /* size_t, NULL */

#include "cbl/memory.h"    /* MEM_ALLOC, MEM_NEW, MEM_FREE */
#include "cbl/assert.h"    /* assert with exception support */
#include "set.h"


#define MAX(x, y) ((x) > (y)? (x): (y))    /* returns larger of two */
#define MIN(x, y) ((x) > (y)? (y): (x))    /* returns smaller of two */


/*
 *  set implemented by hash table
 *
 *  struct set_t contains some information on a set and a hash table to contain set members. A set
 *  looks like the following:
 *
 *      +-----+ -+-
 *      |     |  |  length, timestamp, cmp, hash, size, bucket
 *      |     |  |
 *      |     |  | ------+
 *      +-----+ -+-      | bucket points to the start of hash table
 *      | [ ] | <--------+
 *      | [ ] |
 *      | [ ] |
 *      | [ ] |-[ ]-[ ]  linked list constructed by link member in struct member
 *      | [ ] |
 *      | [ ] |-[ ]-[ ]-[ ]-[ ]
 *      +-----+
 *
 *  The number of buckets in a hash table is determined based on a hint given when set_new() is
 *  invoked. When necessary to find a specific member in a set, cmp() and hash() are used to locate
 *  it. The number of all buckets in a set is necessary when an array is generated to contain all
 *  members in a set (see set_toarray()). timestamp is increased whenever a set is modified by
 *  set_put() or set_remove(). It prevents a user-defined function invoked by set_map() from
 *  modifying a set during traversing it.
 *
 *  Note that a violation of the alignment restriction is possible because there is no guarantee
 *  that the address to which bucket points is aligned properly for a pointer to struct member,
 *  even if such a violation rarely happens in practice. Putting a dummy member of the struct
 *  member pointer type at the beginning of struct set_t can solve the problem.
 *
 *  TODO:
 *    - some of the set operations can be improved if hash numbers are stored in struct member,
 *      which enables a user-provided hashing function called only once for each member and a
 *      user-provided comparison function called only when the hash numbers differ.
 */
struct set_t {
    /* struct binding *dummy; */                 /* dummy member for alignment */
    int size;                                    /* number of buckets in hash table */
    int (*cmp)(const void *x, const void *y);    /* comparison function */
    unsigned (*hash)(const void *x);             /* hash generating function */
    size_t length;                               /* number of members in set */
    unsigned timestamp;                          /* number of modification to set */
    struct member {
        struct member *link;    /* next node in same bucket */
        const void *member;     /* member */
    } **bucket;    /* array of struct member */
};


/*
 *  default function for comparing members
 *
 *  defhashCmp() assumes two given members to be hash strings and compares them by comparing their
 *  addresses for equality (see the hash library for details).
 *
 *  Even if defhashCmp() returns either zero or non-zero for equality, a function provided to
 *  set_new() by a user is supposed to retrun a value less than, equal to or greater than zero to
 *  indicate that x is less than, equal to or greater than y, respectively. See set_new() for more
 *  explanation.
 */
static int defhashCmp(const void *x, const void *y)
{
    return (x != y);
}


/*
 *  default function for generating a hash value
 *
 *  defhashGen() generates a hash value for a hash string (see the hash library for details). It
 *  converts a given pointer value to an unsigned integer and cuts off its two low-order bits
 *  because they are verly likely to be zero due to the alignement restriction. It returns an
 *  integer resulting from converting to a possibly narrow type.
 */
static unsigned defhashGen(const void *x)
{
    return (unsigned long)x >> 2;
}


/*
 *  creates a copy of a set
 *
 *  copy() makes a copy of a set by allocating new storage for it. copy() is used for various set
 *  operations and takes a new hint for the copied set because the size of sets resulting from set
 *  operations may differ from those of the operand sets. See the implementation of set operations
 *  to see how the hint for copy() is used. In order to avoid set_put()'s fruitless search it
 *  directly puts members into a new set.
 */
static set_t *copy(set_t *t, int hint)
{
    set_t *set;

    assert(t);

    set = set_new(hint, t->cmp, t->hash);
    {
        int i;
        struct member *q;

        for (i = 0; i < t->size; i++)
            for (q = t->bucket[i]; q; q = q->link) {
                struct member *p;
                const void *member = q->member;
                int i = set->hash(member) % set->size;

                MEM_NEW(p);
                p->member = member;
                p->link = set->bucket[i];
                set->bucket[i] = p;
                set->length++;
            }
    }

    return set;
}


/*
 *  creates a new set
 *
 *  Even if a function given to cmp should return a value that can distinguish three different
 *  cases for full generality, the current implementation needs only equal or non-equal conditions.
 *
 *  A complicated way to determine the size of the hash table is hired here because the hash number
 *  is computed by a user-provided callback. Compare this to the hash implementation where the size
 *  has a less sophisticated form and the hash number is computed by the library.
 *
 *  The interface for the set library demands more information than a particular implementation of
 *  the interface might require. For example, an implementation using a hash table as given here
 *  need not require the comparison function to distinguish three different cases, and an
 *  implementation using a tree needs no hint and the hashing function passed through hash. They
 *  are all required in the interface to allow for vairous implementation approaches.
 */
set_t *(set_new)(int hint, int cmp(const void *, const void *), unsigned hash(const void *))
{
    /* candidates for size of hash table;
       all are primes probably except INT_MAX */
    static int primes[] = { 509, 509, 1021, 2053, 4093, 8191, 16381, 32771, 65521, INT_MAX };

    int i;
    set_t *set;

    assert(hint >= 0);

    /* i starts with 1; size be largest smaller than hint */
    for (i = 1; primes[i] < hint; i++)    /* no valid range check; hint never larger than INT_MAX */
        continue;

    set = MEM_ALLOC(sizeof(*set) + primes[i-1]*sizeof(set->bucket[0]));
    set->size = primes[i-1];
    set->cmp  = cmp? cmp: defhashCmp;
    set->hash = hash? hash: defhashGen;
    set->length = 0;
    set->timestamp = 0;

    set->bucket = (struct member **)(set + 1);
    for (i = 0; i < set->size; i++)
        set->bucket[i] = NULL;

    return set;
}


/*
 *  inspects if a set contains a member
 */
int (set_member)(set_t *set, const void *member)
{
    int i;
    struct member *p;

    assert(set);
    assert(member);

    i = set->hash(member) % set->size;
    for (p = set->bucket[i]; p; p = p->link)
        if (set->cmp(member, p->member) == 0)
            break;

    return (p != NULL);
}


/*
 *  puts a member to a set
 */
void (set_put)(set_t *set, const void *member)
{
    int i;
    struct member *p;

    assert(set);
    assert(member);

    i = set->hash(member) % set->size;

    for (p = set->bucket[i]; p; p = p->link)
        if (set->cmp(member, p->member) == 0)
            break;

    if (!p) {    /* member not found, so allocate new one */
        MEM_NEW(p);
        p->member = member;
        p->link = set->bucket[i];    /* push to hash table */
        set->bucket[i] = p;
        set->length++;
    } else
        p->member = member;
    set->timestamp++;    /* set modified */
}


/*
 *  removes a member from a set
 */
void *(set_remove)(set_t *set, const void *member)
{
    int i;
    struct member **pp;    /* pointer-to-pointer idiom used */

    assert(set);
    assert(member);

    set->timestamp++;    /* set modified */

    i = set->hash(member) % set->size;

    for (pp = &set->bucket[i]; *pp; pp = &(*pp)->link)
        if (set->cmp(member, (*pp)->member) == 0) {    /* member found */
            struct member *p = *pp;
            *pp = p->link;
            member = p->member;
            MEM_FREE(p);
            set->length--;
            return (void *)member;    /* remove constness */
        }

    /* member not found */
    return NULL;
}


/*
 *  returns the length of a set
 */
size_t (set_length)(set_t *set)
{
    assert(set);

    return set->length;
}


/*
 *  destroys a set
 */
void (set_free)(set_t **pset)
{
    assert(pset);
    assert(*pset);

    if ((*pset)->length > 0) {    /* at least one member */
        int i;
        struct member *p, *q;
        for (i = 0; i < (*pset)->size; i++)
            for (p = (*pset)->bucket[i]; p; p = q) {
                q = p->link;
                MEM_FREE(p);
            }
    }

    MEM_FREE(*pset);
}


/*
 *  calls a user-provided function for each member in a set
 *
 *  timestamp is checked in order to prevent a user-provided function from modifying a set by
 *  calling set_put() or set_remove() that can touch the internal information of a set. If those
 *  functions are called in a callback, it results in assertion failure (which may raise
 *  assert_exceptfail).
 */
void (set_map)(set_t *set, void apply(const void *member, void *cl), void *cl)
{
    int i;
    unsigned stamp;
    struct member *p;

    assert(set);
    assert(apply);

    stamp = set->timestamp;
    for (i = 0; i < set->size; i++)
        for (p = set->bucket[i]; p; p = p->link) {
            apply(p->member, cl);
            assert(set->timestamp == stamp);
        }
}


/*
 *  converts a set to an array
 */
void **(set_toarray)(set_t *set, void *end)
{
    int i, j;
    void **array;
    struct member *p;

    assert(set);

    array = MEM_ALLOC((set->length + 1) * sizeof(*array));

    j = 0;
    for (i = 0; i < set->size; i++)
        for (p = set->bucket[i]; p; p = p->link)
            array[j++] = (void *)p->member;    /* cast removes constness */
    array[j] = end;

    return array;
}


/*
 *  returns a union set of two sets
 *
 *  TODO:
 *    - the code can be modified so that the operation is performed on each pair of corresponding
 *      buckets when two given sets have the same number of buckets.
 */
set_t *(set_union)(set_t *s, set_t *t)
{
    if (!s) {    /* s is empty, t not empty */
        assert(t);
        return copy(t, t->size);
    } else if (!t) {    /* t is empty, s not empty */
        return copy(s, s->size);
    } else {    /* both not empty */
        /* makes copy of s; resulting set has at least as many members as bigger set does */
        set_t *set = copy(s, MAX(s->size, t->size));    /* makes copy of s */

        assert(s->cmp == t->cmp && s->hash == t->hash);    /* same comparison/hashing method */
        {    /* inserts member of t to union set */
            int i;
            struct member *q;
            for (i = 0; i < t->size; i++)
                for (q = t->bucket[i]; q; q = q->link)
                    set_put(set, q->member);
        }

        return set;
    }
}


/*
 *  returns an intersection of two sets
 *
 *  TODO:
 *    - the code can be modified so that the operation is performed on each pair of corresponding
 *      buckets when two given sets have the same number of buckets.
 */
set_t *(set_inter)(set_t *s, set_t *t)
{
    if (!s) {    /* s is empty, t not empty */
        assert(t);
        return set_new(t->size, t->cmp, t->hash);    /* null set */
    } else if (!t) {    /* t is empty, s not empty */
        return set_new(s->size, s->cmp, s->hash);    /* null set */
    } else if (s->length < t->length) {    /* use of <= here results in infinite recursion */
        /* smaller t->size means better performance below, makes t->size smaller */
        return set_inter(t, s);
    } else {    /* both not empty */
        /* resulting set has at most as many members as smaller set does */
        set_t *set = set_new(MIN(s->size, t->size), s->cmp, s->hash);

        assert(s->cmp == t->cmp && s->hash == t->hash);
        {    /* pick member in t and put it to result set if s also has it */
            int i;
            struct member *q;
            for (i = 0; i < t->size; i++)    /* smaller t->size, better performance */
                for (q = t->bucket[i]; q; q = q->link)
                    if (set_member(s, q->member)) {
                        struct member *p;
                        const void *member = q->member;
                        int i = set->hash(member) % set->size;

                        MEM_NEW(p);
                        p->member = member;
                        p->link = set->bucket[i];
                        set->bucket[i] = p;
                        set->length++;
                    }
        }

        return set;
    }
}


/*
 *  returns a difference set of two sets
 *
 *  TODO:
 *    - the code can be modified so that the operation is performed on each pair of corresponding
 *      buckets when two given sets have the same number of buckets.
 */
set_t *(set_minus)(set_t *s, set_t *t) {
    if (!s) {    /* s is empty, t not empty */
        assert(t);
        return set_new(t->size, t->cmp, t->hash);    /* (null set) - t = (null set) */
    } else if (!t) {    /* t is empty, s not empty */
        return copy(s, s->size);    /* s - (null set) = s */
    } else {    /* both not empty */
        set_t *set = set_new(MIN(s->size, t->size), s->cmp, s->hash);

        assert(s->cmp == t->cmp && s->hash == t->hash);
        {    /* pick member in s and put it to result set if t does not has it */
            int i;
            struct member *q;
            for (i = 0; i < s->size; i++)
                for (q = s->bucket[i]; q; q = q->link)
                    if (!set_member(t, q->member)) {
                        struct member *p;
                        const void *member = q->member;
                        int i = set->hash(member) % set->size;

                        MEM_NEW(p);
                        p->member = member;
                        p->link = set->bucket[i];
                        set->bucket[i] = p;
                        set->length++;
                    }
        }

        return set;
    }
}


/*
 *  returns a symmetric difference of two sets
 *
 *  TODO:
 *    - the code can be modified so that the operation is performed on each pair of corresponding
 *      buckets when two given sets have the same number of buckets.
 */

set_t *(set_diff)(set_t *s, set_t *t)
{
    if (!s) {    /* s is empty, t not empty */
        assert(t);
        return copy(t, t->size);    /* (null set)-t U t-(null set) = (null set) U t = t */
    } else if (!t) {    /* t is empty, s not empty */
        return copy(s, s->size);    /* s-(null set) U (null set)-s = s U (null set) = s */
    } else {    /* both not empty */
        set_t *set = set_new(MIN(s->size, t->size), s->cmp, s->hash);

        assert(s->cmp == t->cmp && s->hash == t->hash);
        {    /* set = t - s */
            int i;
            struct member *q;

            for (i = 0; i < t->size; i++)
                for (q = t->bucket[i]; q; q = q->link)
                    if (!set_member(s, q->member)) {
                        struct member *p;
                        const void *member = q->member;
                        int i = set->hash(member) % set->size;

                        MEM_NEW(p);
                        p->member = member;
                        p->link = set->bucket[i];
                        set->bucket[i] = p;
                        set->length++;
                    }
        }

        {    /* set += (s - t) */
            int i;
            struct member *q;

            for (i = 0; i < s->size; i++)
                for (q = s->bucket[i]; q; q = q->link)
                    if (!set_member(t, q->member)) {
                        struct member *p;
                        const void *member = q->member;
                        int i = set->hash(member) % set->size;

                        MEM_NEW(p);
                        p->member = member;
                        p->link = set->bucket[i];
                        set->bucket[i] = p;
                        set->length++;
                    }
        }

        return set;
    }
}

/* end of set.c */
