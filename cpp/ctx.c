/*
 *  context stack for preprocessor
 */

#include <stddef.h>        /* NULL */
#include <stdio.h>         /* FILE, fprintf, putc */
#include <cbl/arena.h>     /* ARENA_ALLOC */
#include <cbl/assert.h>    /* assert */

#include "../src/common.h"
#include "lxl.h"
#include "strg.h"
#include "ctx.h"


static ctx_t base;    /* base context */


ctx_t *ctx_cur = &base;    /* current context */


/*
 *  checks if the current context is the base context
 */
int (ctx_isbase)(void)
{
    assert(ctx_cur);

    return (ctx_cur == &base);
}


/*
 *  pushes a new context into the context stack
 */
ctx_t *(ctx_push)(int type)
{
    ctx_t *p;

    assert(ctx_cur);

    if (!ctx_cur->next) {
        ctx_cur->next = ARENA_ALLOC(strg_perm, sizeof(*ctx_cur->next));
        ctx_cur->next->prev = ctx_cur;
        ctx_cur->next->next = NULL;
    }
    p = ctx_cur->next;

    p->type = type;
    p->list = ctx_cur->list;
    p->cur = ctx_cur->cur;
    ctx_cur = p;

    return p;
}


/*
 *  pops a context from the context stack
 */
void (ctx_pop)(void)
{
    assert(ctx_cur->prev);

    ctx_cur = ctx_cur->prev;
}


/*
 *  initializes the context stack
 */
void (ctx_init)(void)
{
    assert(!base.list);

    base.list = lxl_new(strg_perm);
    base.cur = base.list->head;
}


/*
 *  prints the current context for debugging
 */
void (ctx_print)(FILE *fp)
{
    ctx_t *p;
    const char *name[] = { "norm", "peek", "ignore" };

    assert(fp);
    assert(ctx_cur);

    for (p = ctx_cur; p; p = p->prev) {
        assert(p->type < NELEM(name));
        fprintf(fp, "[%8p] %s\n", (void *)p, name[p->type]);
        lxl_print(p->list, p->cur, fp);
        putc('\n', fp);
    }
}

/* end of ctx.c */
