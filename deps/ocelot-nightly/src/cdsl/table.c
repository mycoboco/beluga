/*
 *  table (cdsl)
 */

#include <limits.h>    /* INT_MAX */
#include <stddef.h>    /* size_t, NULL */

#include "cbl/memory.h"    /* MEM_ALLOC, MEM_FREE */
#include "cbl/assert.h"    /* assert with exception support */
#include "table.h"


/*
 *  table implemented by hash table
 *
 *  struct table_t contains information on a table and a hash table to contain key-value pairs. A
 *  table looks like the following:
 *
 *      +-----+ -+-
 *      |     |  |  size, cmp, hash, length, bucket
 *      |     |  |
 *      |     |  | ------+
 *      +-----+ -+-      | bucket points to the start of hash table
 *      | [ ] | <--------+
 *      | [ ] |
 *      | [ ] |
 *      | [ ] |-[ ]-[ ]  linked list constructed by link member in struct binding
 *      | [ ] |
 *      | [ ] |-[ ]-[ ]-[ ]-[ ]
 *      +-----+
 *
 *  The number of buckets in a hash table is determined based on a hint given when table_new() is
 *  invoked. When necessary to find a specific key in a table, cmp() and hash() are used to locate
 *  it. The number of all buckets in a table is necessary when an array is generated to contain all
 *  key-value pairs in a table (see table_toarray()). timestamp is increased whenever a table is
 *  modified by table_put() or table_remove(). It prevents a user-defined function invoked by
 *  table_map() from modifying a table during traversing it.
 *
 *  Note that a violation of the alignment restriction is possible because there is no guarantee
 *  that the address to which bucket points is aligned properly for a pointer to struct binding,
 *  even if such a violation rarely happens in practice. Putting a dummy member of the struct
 *  binding pointer type at the beginning of struct table_t can solve the problem.
 */
struct table_t {
    /* struct binding *dummy; */               /* dummy member for alignment */
    int size;                                  /* number of buckets in hash table */
    int (*cmp)(const void *, const void *);    /* comparison function */
    unsigned (*hash)(const void *);            /* hash generation function */
    size_t length;                             /* number of key-value pairs in table */
    unsigned timestamp;                        /* number of modification to table */
    struct binding {
        struct binding *link;    /* next node in same bucket */
        const void *key;         /* key */
        void *value;             /* value */
    } **bucket;    /* array of struct binding */
};


/*
 *  default function for comparing keys
 *
 *  defhashCmp() assumes two given keys to be hash strings and compares them by comparing their
 *  addresses for equality (see the hash library for details).
 *
 *  Even if defhashCmp() returns either zero or non-zero for equality, a function provided to
 *  table_new() by a user is supposed to retrun a value less than, equal to or greater than zero to
 *  indicate that x is less than, equal to or greater than y, respectively. See table_new() for
 *  more explanation.
 */
static int defhashCmp(const void *x, const void *y)
{
    return (x != y);
}


/*
 *  default function for generating a hash value
 *
 *  defhashGen() generates a hash value for a hash string (see the hash library for details). It
 *  converts a pointer value to an unsigned integer and cuts off its two low-order bits because
 *  they are verly likely to be zero due to the alignement restriction. It returns an integer
 *  resulting from converting to a possibly narrow type.
 */
static unsigned defhashGen(const void *key)
{
    return (unsigned long)key >> 2;
}


/*
 *  creates a new table
 *
 *  Even if a function given to cmp should return a value that can distinguish three different
 *  cases for full generality, the current implementation needs only equal or non-equal conditions.
 *
 *  A complicated way to determine the size of the hash table is hired here because the hash number
 *  is computed by a user-provided callback. Compare this to the hash library implementation where
 *  the size has a less sophisticated form and the hash number is computed by the library.
 *
 *  The interface for the table library demands more information than a particular implementation
 *  of the interface might require. For example, an implementation using a hash table as given here
 *  need not require the comparison function to distinguish three different cases, and an
 *  implementation using a tree does not need hint and the hashing function passed through hash.
 *  They are all required in the interface to allow for vairous implementation approaches.
 */
table_t *(table_new)(int hint, int cmp(const void *, const void *), unsigned hash(const void *))
{
    /* candidates for size of hash table;
       all are primes probably except INT_MAX */
    static int prime[] = { 509, 509, 1021, 2053, 4093, 8191, 16381, 32771, 65521, INT_MAX };

    int i;
    table_t *table;

    assert(hint >= 0);

    /* i starts with 1; size be largest smaller than hint */
    for (i = 1; prime[i] < hint; i++)    /* no valid range check; hint never larger than INT_MAX */
        continue;

    table = MEM_ALLOC(sizeof(*table) + prime[i-1]*sizeof(table->bucket[0]));
    table->size = prime[i-1];
    table->cmp = cmp? cmp: defhashCmp;
    table->hash = hash? hash: defhashGen;
    table->length = 0;
    table->timestamp = 0;

    table->bucket = (struct binding **)(table + 1);
    for (i = 0; i < table->size; i++)
        table->bucket[i] = NULL;

    return table;
}


/*
 *  gets data for a key from a table
 */
void *(table_get)(const table_t *table, const void *key)
{
    int i;
    struct binding *p;

    assert(table);
    assert(key);

    i = table->hash(key) % table->size;
    for (p = table->bucket[i]; p; p = p->link)
        if (table->cmp(key, p->key) == 0)    /* same key found */
            break;

    return p? p->value: NULL;
}


/*
 *  puts a value for a key to a table
 */
void *(table_put)(table_t *table, const void *key, void *value)
{
    int i;
    struct binding *p;
    void *prev;

    assert(table);
    assert(key);

    i = table->hash(key) % table->size;

    for (p = table->bucket[i]; p; p = p->link)
        if ((*table->cmp)(key, p->key) == 0)
            break;

    if (!p) {    /* key not found, so allocate new one */
        MEM_NEW(p);
        p->key = key;
        p->link = table->bucket[i];    /* push to hash table */
        table->bucket[i] = p;
        table->length++;
        prev = NULL;    /* no previous value */
    } else
        prev = p->value;

    p->value = value;
    table->timestamp++;    /* table modified */

    return prev;
}


/*
 *  returns the length of a table
 */
size_t (table_length)(const table_t *table)
{
    assert(table);

    return table->length;
}


/*
 *  calls a user-provided function for each key-value pair in a table
 *
 *  timestamp is checked in order to prevent a user-provided function from modifying a table by
 *  calling table_put() or table_remove() that can touch the internal information of a table. If
 *  those functions are called in a callback, it results in assertion failure (which may raise
 *  assert_exceptfail).
 *
 *  TODO:
 *    - it sometimes serves better to call a user callback for key-value pairs in order they are
 *      stored.
 */
void (table_map)(table_t *table, void apply(const void *key, void **value, void *cl), void *cl)
{
    int i;
    unsigned stamp;
    struct binding *p;

    assert(table);
    assert(apply);

    stamp = table->timestamp;
    for (i = 0; i < table->size; i++)
        for (p = table->bucket[i]; p; p = p->link) {
            apply(p->key, &p->value, cl);
            assert(table->timestamp == stamp);
        }
}


/*
 *  removes a key-value pair from a table
 */
void *(table_remove)(table_t *table, const void *key)
{
    int i;
    struct binding **pp;    /* pointer-to-pointer idiom used */

    assert(table);
    assert(key);

    table->timestamp++;    /* table modified */

    i = table->hash(key) % table->size;

    for (pp = &table->bucket[i]; *pp; pp = &(*pp)->link)
        if (table->cmp(key, (*pp)->key) == 0) {    /* key found */
            struct binding *p = *pp;
            void *value = p->value;
            *pp = p->link;
            MEM_FREE(p);
            table->length--;
            return value;
        }

    /* key not found */
    return NULL;
}


/*
 *  converts a table to an array
 *
 *  TODO:
 *    - it sometimes serves better to call a user callback for key-value pairs in order they are
 *      stored.
 */
void **(table_toarray)(const table_t *table, void *end)
{
    int i, j;
    void **array;
    struct binding *p;

    assert(table);

    array = MEM_ALLOC((2*table->length + 1) * sizeof(*array));

    j = 0;
    for (i = 0; i < table->size; i++)
        for (p = table->bucket[i]; p; p = p->link) {
            array[j++] = (void *)p->key;    /* cast removes constness */
            array[j++] = p->value;
        }
    array[j] = end;

    return array;
}


/*
 *  destroys a table
 */
void (table_free)(table_t **ptable)
{
    assert(ptable);
    assert(*ptable);

    if ((*ptable)->length > 0) {    /* at least one pair */
        int i;
        struct binding *p, *q;
        for (i = 0; i < (*ptable)->size; i++)
            for (p = (*ptable)->bucket[i]; p; p = q) {
                q = p->link;
                MEM_FREE(p);
            }
    }

    MEM_FREE(*ptable);
}

/* end of table.c */
