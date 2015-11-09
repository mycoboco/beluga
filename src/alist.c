/*
 *  arean linked-list
 */

#include <stddef.h>        /* size_t, NULL */
#include <cbl/arena.h>     /* arena_t, ARENA_ALLOC */
#include <cbl/assert.h>    /* assert */

#include "alist.h"


/*
 *  appends a new node to a list;
 *  NULL is considered an empty list
 */
alist_t *(alist_append)(alist_t *list, void *data, arena_t *a)
{
    alist_t *node;

    node = ARENA_ALLOC(a, sizeof(*node));
    if (list) {
        node->next = list->next;
        list->next = node;
    } else
        node->next = node;
    node->data = data;

    return node;
}


/*
 *  counts the number of nodes in a list
 */
size_t (alist_length)(const alist_t *list)
{
    size_t n = 0;

    if (list) {
        const alist_t *p = list;
        do
            n++;
        while((p=p->next) != list);
    }

    return n;
}


/*
 *  converts a list to a null-terminated array
 */
void **(alist_toarray)(const alist_t *list, arena_t *a)
{
    void **array;
    size_t i, n;

    n = alist_length(list);
    array = ARENA_ALLOC(a, (n+1)*sizeof(*array));
    for (i = 0; i < n; i++) {
        list = list->next;
        array[i] = list->data;
    }
    array[i] = NULL;

    return array;
}

/* end of alist.c */
