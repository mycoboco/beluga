/*
 *  stack (cdsl)
 */

#include <stddef.h>    /* NULL */

#include "cbl/assert.h"    /* assert with exception support */
#include "cbl/memory.h"    /* MEM_NEW, MEM_FREE */
#include "stack.h"


/* stack implemented by linked list */
struct stack_t {
    struct node {
        void *data;           /* data contained in node */
        struct node *next;    /* next node */
    } *head;                  /* head node of stack */
};


/*
 *  creates a stack
 */
stack_t *(stack_new)(void)
{
    stack_t *stk;

    MEM_NEW(stk);
    stk->head = NULL;

    return stk;
}


/*
 *  inspects if a stack is empty
 */
int (stack_empty)(const stack_t *stk)
{
    assert(stk);
    return (!stk->head);
}


/*
 *  pushes data into a stack
 */
void (stack_push)(stack_t *stk, void *data)
{
    struct node *t;

    assert(stk);
    MEM_NEW(t);
    t->data = data;
    t->next = stk->head;
    stk->head = t;
}


/*
 *  pops data from a stack
 */
void *(stack_pop)(stack_t *stk)
{
    void *data;
    struct node *t;

    assert(stk);
    assert(stk->head);

    t = stk->head;
    stk->head = t->next;
    data = t->data;
    MEM_FREE(t);

    return data;
}


/*
 *  peeks the top-most data in a stack
 */
void *(stack_peek)(const stack_t *stk)
{
    assert(stk);
    assert(stk->head);

    return (void *)((struct node *)stk->head)->data;
}


/*
 *  destroys a stack
 */
void (stack_free)(stack_t **stk)
{
    struct node *t, *u;

    assert(stk);
    assert(*stk);

    for (t = (*stk)->head; t; t = u) {
        u = t->next;
        MEM_FREE(t);
    }
    MEM_FREE(*stk);
}

/* end of stack.c */
