/*
 *  doubly-linked list (cdsl)
 */

#include <limits.h>    /* LONG_MAX */
#include <stddef.h>    /* NULL */
#include <stdarg.h>    /* va_start, va_arg, va_end, va_list */

#include "cbl/assert.h"    /* assert with exception support */
#include "cbl/memory.h"    /* MEM_NEW0, MEM_FREE, MEM_NEW */
#include "dlist.h"


/*
 *  doubly-linked list (a.k.a. ring)
 */
struct dlist_t {
    struct node {
        struct node *prev;    /* previous node */
        struct node *next;    /* next node */
        void *data;           /* data */
    } *head;                  /* start of list */
    long length;              /* length of list (number of nodes) */
    long lastidx;             /* index of last accessed node */
    struct node *lastnode;    /* last accessed node */
};


/*
 *  creates an empty new list
 */
dlist_t *(dlist_new)(void)
{
    dlist_t *dlist;

    MEM_NEW0(dlist);
    dlist->head = NULL;

    dlist->lastidx = 0;
    dlist->lastnode = NULL;

    return dlist;
}


/*
 *  constructs a new list using a given sequence of data
 */
dlist_t *(dlist_list)(void *data, ...)
{
    va_list ap;
    dlist_t *dlist = dlist_new();

    va_start(ap, data);
    /* if data is null, following loop skipped and just dlist_new() returned */
    for (; data; data = va_arg(ap, void *))
        dlist_addtail(dlist, data);    /* dlist_addtail() does dirty job */
    va_end(ap);

    return dlist;
}


/*
 *  destroys a list
 */
void (dlist_free)(dlist_t **pdlist)
{
    struct node *p,    /* node to be freed */
                *q;    /* next to node freed */

    assert(pdlist);
    assert(*pdlist);

    if ((p = (*pdlist)->head) != NULL) {
        long n = (*pdlist)->length;
        for (; n-- > 0; p = q) {
            q = p->next;
            MEM_FREE(p);
        }
    }
    MEM_FREE(*pdlist);
}


/*
 *  returns the length of a list
 */
long (dlist_length)(const dlist_t *dlist)
{
    assert(dlist);
    return dlist->length;
}


/*
 *  retrieves data stored in the i-th node in a list
 */
void *(dlist_get)(dlist_t *dlist, long i)
{
    long n;
    struct node *q;

    assert(dlist);
    assert(i >= 0 && i < dlist->length);

    q = NULL;

    if (dlist->lastnode) {    /* tries to use last access information */
        if (dlist->lastidx == i)
            q = dlist->lastnode;
        else if (dlist->lastidx == i - 1 || (i == 0 && dlist->lastidx == dlist->length))
            q = dlist->lastnode->next;
        else if (dlist->lastidx - 1 == i || (dlist->lastidx == 0 && i == dlist->length))
            q = dlist->lastnode->prev;
    }

    if (!q) {
        q = dlist->head;
        if (i <= dlist->length / 2)
            for (n = i; n-- > 0; q = q->next)
                continue;
        else    /* ex: length==5 && i==3 => n=2 */
            for (n = dlist->length-i; n-- > 0; q = q->prev)
                continue;
    }

    dlist->lastidx = i;
    dlist->lastnode = q;

    return q->data;
}


/*
 *  replaces data stored in a node with new given data
 */
void *(dlist_put)(dlist_t *dlist, long i, void *data)
{
    long n;
    void *prev;
    struct node *q;

    assert(dlist);
    assert(i >= 0 && i < dlist->length);

    q = NULL;

    if (dlist->lastnode) {    /* tries to use last access information */
        if (dlist->lastidx == i)
            q = dlist->lastnode;
        else if (dlist->lastidx == i - 1 || (i == 0 && dlist->lastidx == dlist->length))
            q = dlist->lastnode->next;
        else if (dlist->lastidx - 1 == i || (dlist->lastidx == 0 && i == dlist->length))
            q = dlist->lastnode->prev;
    }

    if (!q) {
        q = dlist->head;
        if (i <= dlist->length / 2)
            for (n = i; n-- > 0; q = q->next)
                continue;
        else
            for (n = dlist->length-i; n-- > 0; q = q->prev)
                continue;
    }

    prev = q->data;
    q->data = data;

    return prev;
}


/*
 *  adds a node after the last node
 */
void *(dlist_addtail)(dlist_t *dlist, void *data)
{
    struct node *p,    /* new node */
                *head;

    assert(dlist);
    assert(dlist->length < LONG_MAX);

    MEM_NEW(p);
    if ((head = dlist->head) != NULL) {    /* there is at least one node */
        p->prev = head->prev;    /* new->prev = head->prev == tail */
        head->prev->next = p;    /* head->prev->next == tail->next = new */
        p->next = head;          /* new->next = head */
        head->prev = p;          /* head->prev = new */
    } else    /* empty list */
        dlist->head = p->prev = p->next = p;

    dlist->length++;
    p->data = data;

    return data;
}


/*
 *  adds a new node before the head node
 */
void *(dlist_addhead)(dlist_t *dlist, void *data)
{
    assert(dlist);

    dlist_addtail(dlist, data);
    dlist->head = dlist->head->prev;    /* moves head node */

    dlist->lastidx++;    /* no need to check dlist->lastnode */

    return data;
}


/*
 *  adds a new node to a specified position in a list
 */
void *(dlist_add)(dlist_t *dlist, long pos, void *data)
{
    assert(dlist);
    assert(pos >= -dlist->length);
    assert(pos <= dlist->length+1);
    assert(dlist->length < LONG_MAX);

    /* note that first branch below deals with empty list case; code revised to call dlist_addtail()
       rather than dlist_addhead() because former is simpler even if both do same */
    if (pos == 0 || pos == dlist->length+1)
        return dlist_addtail(dlist, data);
    else if (pos == 1 || pos == -dlist->length)
        return dlist_addhead(dlist, data);
    else {    /* inserting node to middle of list */
        long i;
        struct node *p,    /* new node */
                    *q;    /* node to be next to new node */

        /* find index of node that will become next of new node;
           if pos < 0, pos+(length+1) == positive value for same position */
        pos = (pos < 0)? pos+(dlist->length+1)-1: pos-1;

        q = NULL;

        if (dlist->lastnode) {    /* tries to use last access information */
            if (dlist->lastidx == pos)
                q = dlist->lastnode;
            else if (dlist->lastidx == pos - 1 || (pos == 0 && dlist->lastidx == dlist->length))
                q = dlist->lastnode->next;
            else if (dlist->lastidx - 1 == pos || (dlist->lastidx == 0 && pos == dlist->length))
                q = dlist->lastnode->prev;

            /* adjusts last access information */
            if (dlist->lastidx >= pos)
                dlist->lastidx++;
        }

        if (!q) {
            q = dlist->head;
            if (pos <= dlist->length / 2)
                for (i = pos; i-- > 0; q = q->next)
                    continue;
            else
                for (i = dlist->length-pos; i-- > 0; q = q->prev)
                    continue;
        }

        MEM_NEW(p);
        p->prev = q->prev;    /* new->prev = to_be_next->prev */
        q->prev->next = p;    /* to_be_next->prev->next == to_be_prev->next = new */
        p->next = q;          /* new->next = to_be_next */
        q->prev = p;          /* to_be_next->prev = new */
        dlist->length++;

        p->data = data;

        return data;
    }
}


/*
 *  removes a node with a specific index from a list
 */
void *(dlist_remove)(dlist_t *dlist, long i)
{
    long n;
    void *data;
    struct node *q;    /* node to be removed */

    assert(dlist);
    assert(dlist->length > 0);
    assert(i >= 0 && i < dlist->length);

    q = NULL;

    if (dlist->lastnode) {    /* tries to use last access information */
        if (dlist->lastidx == i)
            q = dlist->lastnode;
        else if (dlist->lastidx == i - 1 || (i == 0 && dlist->lastidx == dlist->length))
            q = dlist->lastnode->next;
        else if (dlist->lastidx - 1 == i || (dlist->lastidx == 0 && i == dlist->length))
            q = dlist->lastnode->prev;

        /* adjusts last access information */
        if (dlist->lastidx > i)
            dlist->lastidx--;
        else if (dlist->lastidx == i) {
            dlist->lastnode = dlist->lastnode->next;
            if (dlist->lastidx == dlist->length - 1)
                dlist->lastidx = 0;
        }
    }

    if (!q) {
        q = dlist->head;
        if (i <= dlist->length/2)
            for (n = i; n-- > 0; q = q->next)
                continue;
        else
            for (n = dlist->length-i; n-- > 0; q = q->prev)
                continue;
    }

    if (i == 0)    /* remove head */
        dlist->head = dlist->head->next;
    data = q->data;
    q->prev->next = q->next;
    q->next->prev = q->prev;
    MEM_FREE(q);
    if (--dlist->length == 0)    /* empty */
        dlist->head = dlist->lastnode = NULL;    /* also invalidates dlist->lastnode */

    return data;
}


/*
 *  removes the last node of a list
 *
 *  Note that the last access information is invalidated only when it refers to the tail node which
 *  will get removed. In such a case, moving lastnode to the tail's next or previous node is not
 *  necessary because it will be the head or the tail after the removal and an access to the head
 *  or the tail node does not entail performance degradation.
 */
void *(dlist_remtail)(dlist_t *dlist)
{
    void *data;
    struct node *tail;

    assert(dlist);
    assert(dlist->length > 0);

    if (dlist->lastnode == dlist->head->prev)    /* implies dlist->lastnode != NULL */
        dlist->lastnode = NULL;    /* invalidates dlist->lastnode */

    tail = dlist->head->prev;
    data = tail->data;
    tail->prev->next = tail->next;    /* tail->next == head */
    tail->next->prev = tail->prev;    /* tail->next->prev == head->prev */
    MEM_FREE(tail);
    if (--dlist->length == 0)    /* empty */
        dlist->head = NULL;

    return data;
}


/*
 *  removes the first node from a list
 */
void *(dlist_remhead)(dlist_t *dlist)
{
    assert(dlist);
    assert(dlist->length > 0);

    /* dlist->lastnode adjusted in dlist_remtail() */

    dlist->head = dlist->head->prev;    /* turns head to tail */

    return dlist_remtail(dlist);
}


/*
 *  shifts a list to right or left
 */
void (dlist_shift)(dlist_t *dlist, long n)
{
    long i;
    struct node *q;    /* new head node after shift */

    assert(dlist);
    assert(n >= -dlist->length);
    assert(n <= dlist->length);

    /* following adjustment to i differs from original code */
    if (n >= 0)    /* shift to right: head goes to left */
        i = dlist->length - n;
    else    /* shift to left; head goes to right */
        i = -n;    /* possibility of overflow in 2sC representation */

    q = NULL;

    if (dlist->lastnode) {    /* tries to use last access information */
        if (dlist->lastidx == i)
            q = dlist->lastnode;
        else if (dlist->lastidx == i - 1 || (i == 0 && dlist->lastidx == dlist->length))
            q = dlist->lastnode->next;
        else if (dlist->lastidx - 1 == i || (dlist->lastidx == 0 && i == dlist->length))
            q = dlist->lastnode->prev;

        /* adjusts last access information */
        if (dlist->lastidx < i)
            dlist->lastidx += (dlist->length - i);
        else
            dlist->lastidx -= i;
    }

    if (!q) {
        q = dlist->head;
        if (i <= dlist->length/2)
            for (n = i; n-- > 0; q = q->next)
                continue;
        else
            for (n = dlist->length-i; n-- > 0; q = q->prev)
                continue;
    }

    dlist->head = q;
}

/* end of dlist.c */
