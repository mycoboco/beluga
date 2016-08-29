/*
 *  token list
 */

#include <cbl/assert.h>    /* assert */
#ifndef NDEBUG
#include <stdio.h>         /* FILE */
#endif    /* !NDEBUG */

#include "lex.h"
#include "proc.h"

/* helper to manipulate token lists */
#define SWAP(x, y)               \
    do {                         \
        lex_t *tmp = (x);        \
        (x) = (y), (y) = tmp;    \
    } while(0)


/* internal functions referenced forwardly */
static lex_t *nextf(void);
static lex_t *nexti(void);


lex_t *lst_in, *lst_out;              /* input/output token list */
lex_t *(*lst_nexti)(void) = nextf;    /* retrieves token from input list */


static lex_t *cur;    /* token being processed in input list */


/*
 *  appends tok to head
 */
lex_t *(lst_append)(lex_t *head, lex_t *tok)
{
    assert(tok);

    if (!head)
        return tok;
    SWAP(tok->next, head->next);

    return tok;
}


/*
 *  inserts tok after cur in head
 */
lex_t *(lst_insert)(lex_t *head, lex_t *cur, lex_t *tok)
{
    assert(head);
    assert(cur);
    assert(tok);

    SWAP(tok->next, head->next);

    return (head == cur)? tok: head;
}


/*
 *  flushes the input list to the output list
 */
void (lst_flush)(void)
{
    assert(cur);    /* implies assert(lst_in) */

    if (cur == lst_in)
        lst_in = NULL;
    else
        SWAP(lst_in->next, cur->next);
    lst_out = lst_append(lst_out, cur);
    cur = NULL;

    lst_nexti = nextf;
}


/*
 *  discards the input list up to cur (inclusive)
 */
void (lst_discard)(void)
{
    lex_t *p;
    void *q;

    assert(cur);    /* implies assert(lst_in) */

    for (p = lst_in->next; p != cur; p = p->next) {
        if (p->f.alloc) {
            q = (void *)p->spell;
            MEM_FREE(q);
        }
        q = p;
        MEM_FREE(q);
    }
    if (lst_in == cur)
        lst_in = NULL;
    if (cur->f.alloc) {
        q = (void *)cur->spell;
        MEM_FREE(q);
    }
    MEM_FREE(cur);

    lst_nexti = nextf;
}


/*
 *  fills the input list after flush or discard
 */
static lex_t *nextf(void)
{
    assert(!cur);

    if (lst_in)
        cur = lst_in;
    else
        lst_in = cur = lex_next();

    lst_nexti = nexti;
    return cur;
}


/*
 *  retrieves a token from the input list
 */
static lex_t *nexti(void)
{
    assert(cur);    /* implies assert(lst_in) */

    if (cur == lst_in)
        lst_in = lst_append(lst_in, lex_next());
    cur = cur->next;

    return cur;
}


/*
 *  retrieves a token from the output list
 */
lex_t *(lst_next)(void)
{
    lex_t *t;

    assert(lst_out);

    if (lst_out == lst_out->next)    /* only one */
        proc_prep();
    t = lst_out->next;
    lst_out->next = lst_out->next->next;

    return t;
}


#ifndef NDEBUG
/*
 *  prints a token list for debugging
 */
void (lst_print)(lex_t *p, FILE *fp)
{
    lex_t *q;

    if (!p)
        return;

    q = p = p->next;
    do {
        fprintf(fp, "%c %d: %s\n", (p == cur)? '*': '-', p->id, p->spell);
        p = p->next;
    } while(p != q);
}
#endif    /* NDEBUG */

/* end of lst.c */
