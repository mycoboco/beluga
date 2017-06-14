/*
 *  list (cdsl)
 */

#include <stdarg.h>    /* va_list, va_start, va_arg, va_end */
#include <stddef.h>    /* NULL, size_t */

#include "cbl/assert.h"    /* assert with exception support */
#include "cbl/memory.h"    /* MEM_NEW, MEM_FREE, MEM_ALLOC */
#include "list.h"


/*
 *  pushes a new node to a list
 */
list_t *(list_push)(list_t *list, void *data)
{
    list_t *p;

    MEM_NEW(p);
    p->data = data;
    p->next = list;

    return p;
}


/*
 *  constructs a new list using a sequence of data
 */
list_t *(list_list)(void *data, ...)
{
    va_list ap;
    list_t *list,
           **plist = &list;    /* double pointer into which next data is to be stored */

    va_start(ap, data);
    /* if data is null, following loop skipped and null returned */
    for (; data; data = va_arg(ap, void *)) {
        MEM_NEW(*plist);
        (*plist)->data = data;
        plist = &(*plist)->next;
    }
    *plist = NULL;    /* next of last node should be null */
    va_end(ap);

    return list;
}


#if 0    /* non-pointer-to-pointer version */
list_t *(list_list)(void *data, ...)
{
    va_list ap;
    list *list;

    /* consider three cases:
           1) empty list
           2) list has only one node
           3) list has two or more nodes */

    va_start(ap, data);
    list = NULL;
    for (; data; data = va_arg(ap, void *)) {
        if (!list) {    /* empty list */
            MEM_NEW(list);
        } else {    /* one or more nodes exist */
            MEM_NEW(list->next);
            list = list->next;
        }
        list->data = data;
    }
    if (list)    /* at least one node */
        list->next = NULL;
    va_end(ap);

    return list;
}
#endif    /* disabled */


/*
 *  appends a list to another
 *
 *  TODO:
 *    - the time complexity of the current implementation is O(N) where N indicates the number of
 *      nodes in a list. With a circular list, where the next node of the last node set to the
 *      head, it is possible for both pushing and appending to be done in a constant time.
 */
list_t *(list_append)(list_t *list, list_t *tail)
{
    list_t **p = &list;    /* double pointer whose next set to point to tail */

    while (*p)    /* find last node */
        p = &(*p)->next;
    *p = tail;    /* appending */

    return list;
}


#if 0    /* non-pointer-to-pointer version */
list_t *(list_append)(list_t *list, list_t *tail)
{
    /* consider four cases:
           1) empty + empty
           2) empty + one or more nodes
           3) one or more nodes + empty
           4) one or more nodes + one or more nodes */

    if (!list)    /* empty + tail */
        list = tail;
    else {    /* non-empty + tail */
        while (list->next)
            list = list->next;
        list->next = tail;
    }

    return list;
}
#endif    /* disabled */


/*
 *  duplicates a list
 */
list_t *(list_copy)(const list_t *list)
{
    list_t *head,
           **plist = &head;    /* double pointer into which node copied */

    for (; list; list = list->next) {
        MEM_NEW(*plist);
        (*plist)->data = list->data;
        plist = &(*plist)->next;
    }
    *plist = NULL;    /* next of last node should be null */

    return head;
}


#if 0    /* non-pointer-to-pointer version */
list_t *(list_copy)(const list_t *list)
{
    list_t *head, *tlist;

    head = NULL;
    for (; list; list = list->next) {
        if (!head) {    /* for first node */
            MEM_NEW(head);
            tlist = head;
        } else {    /* for rest */
            MEM_NEW(tlist->next);
            tlist = tlist->next;
        }
        tlist->data = list->data;
    }
    if (head)    /* at least one node */
        tlist->next = NULL;

    return head;
}
#endif    /* disabled */


/*
 *  pops a node from a list and save its data into a pointer object
 */
list_t *(list_pop)(list_t *list, void **pdata)
{
    if (list) {
        list_t *head = list->next;
        if (pdata)
            *pdata = list->data;
        MEM_FREE(list);
        return head;
    } else
        return list;
}


/*
 *  reverses a list
 */
list_t *(list_reverse)(list_t *list)
{
    list_t *head, *next;

    head = NULL;
    for (; list; list = next) {
        next = list->next;
        list->next = head;
        head = list;
    }

    return head;
}


#if 0    /* pointer-to-pointer version */
list_t *(list_reverse)(list_t *list)
{
    list_t *head, *next, **plist;

    /* basic idea is to repeat: pick head and push to another list where
                                    list: picked head
                                    next: head after moving head
                                    head: head of another list */

    head = NULL;
    while (list) {
        /* perpare to move head */
        plist = &list->next;    /* A */
        next = list->next;
        /* make head another list's head */
        *plist = head;          /* B */
        head = list;
        /* pick next node */
        list = next;
    }

    /* no modification to list->next between A and B, so plist is unnecessary;
       removing it and using for statement result in original version above */

    return head;
}
#endif    /* disabled */


/*
 *  counts the length of a list
 */
size_t (list_length)(const list_t *list)
{
    size_t n;

    for (n = 0; list; list = list->next)
        n++;

    return n;
}


/*
 *  destroys a list
 */
void (list_free)(list_t **plist)
{
    list_t *next;

    assert(plist);
    for (; *plist; *plist = next) {
        next = (*plist)->next;
        MEM_FREE(*plist);
    }
}


/*
 *  calls a user-provided function for each node in a list
 */
void (list_map)(list_t *list, void apply(void **, void *), void *cl)
{
    assert(apply);

    for (; list; list = list->next)
        apply(&list->data, cl);
}


/*
 *  converts a list to an array
 */
void **(list_toarray)(const list_t *list, void *end)
{
    void **array;
    size_t i, n;

    n = list_length(list);
    array = MEM_ALLOC((n+1)*sizeof(*array));
    for (i = 0; i < n; i++) {
        array[i] = list->data;
        list = list->next;
    }
    array[i] = end;

    return array;
}

/* end of list.c */
