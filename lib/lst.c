/*
 *  token list
 */

#include <stddef.h>        /* size_t, NULL */
#include <string.h>        /* memcpy */
#include <cbl/arena.h>     /* arena_t, ARENA_ALLOC */
#include <cbl/assert.h>    /* assert */
#include <cdsl/hash.h>     /* hash_string */
#ifndef NDEBUG
#include <stdio.h>         /* FILE, fprintf, fputs */
#endif    /* !NDEBUG */

#include "in.h"
#include "lex.h"
#include "mcr.h"
#include "proc.h"
#include "strg.h"
#include "lst.h"

/* helper to manipulate token lists */
#define SWAP(x, y)               \
    do {                         \
        lex_t *tmp = (x);        \
        (x) = (y), (y) = tmp;    \
    } while(0)


/* context for input lists */
struct ctx {
    lex_t *in, *out;     /* input/output token list */
    lex_t *cur;          /* token being processed in input list */
    struct ctx *prev;    /* previous context */
    struct ctx *next;    /* next context */
};


/* internal functions referenced forwardly */
static lex_t *nexti(void);
static lex_t *nextl(void);
static lex_t *peeki(void);
static lex_t *peekl(void);


lex_t *(*lst_nexti)(void) = nexti;    /* retrieves token from input list */
lex_t *(*lst_peeki)(void) = peeki;    /* looks ahead next token for function-like macros */


static struct ctx base;            /* base context */
static struct ctx *ctx = &base;    /* current context */
static lex_t eoi = {               /* EOI token */
    LEX_EOI,
    "",
    NULL,
    { 0, 1, 0, 0 },
    &eoi
};


/*
 *  pushes a new context into the context stack
 */
void (lst_push)(lex_t *in)
{
    struct ctx *p;

    if (!ctx->next) {
        ctx->next = ARENA_ALLOC(strg_perm, sizeof(*ctx->next));
        ctx->next->prev = ctx;
        ctx->next->next = NULL;
    }
    p = ctx->next;

    p->in = in;
    p->out = p->cur = NULL;
    lst_nexti = nextl;
    lst_peeki = peekl;
    ctx = p;
}


/*
 *  pops a context from the context stack
 */
lex_t *(lst_pop)(void)
{
    assert(ctx->prev);

    ctx = ctx->prev;
    if (ctx == &base) {
        lst_nexti = nexti;
        lst_peeki = peeki;
    }

    return ctx->next->out;
}


/*
 *  appends a token/list to a list
 */
lex_t *(lst_append)(lex_t *l, lex_t *t)
{
    assert(t);

    if (!l)
        return t;

    SWAP(t->next, l->next);

    return t;
}


/*
 *  inserts a token list after the current input token
 */
void (lst_insert)(lex_t *l)
{
    lex_t *p;

    assert(l);

    if (!ctx->in)
        ctx->in = l;
    else {
        p = (ctx->cur)? ctx->cur: ctx->in;
        SWAP(l->next, p->next);
        if (ctx->in == ctx->cur)
            ctx->in = l;
    }
}


/*
 *  flushes to the current output list
 */
void (lst_flush)(int nested, int inc)
{
    static int cnt;

    lex_t *p, *q;

    assert(ctx == &base || nested);
    assert(ctx->cur);                  /* implies assert(ctx->in) */

    q = ctx->in;
    if (inc) {
        p = ctx->cur;
        if (p == q) {
            if (ctx == &base && ++cnt == 50) {
                arena_t *a = strg_line;
                strg_get();
                ctx->out = lst_append(ctx->out, lex_make(-1, (char *)a, 0));
                cnt = 0;
            }
            ctx->in = NULL;
        }
        ctx->cur = NULL;
    } else {
        p = ctx->in->next;
        while (p->next != ctx->cur)
            p = p->next;
        if (p == q)
            return;
    }
    SWAP(p->next, q->next);
    ctx->out = lst_append(ctx->out, p);
}


/*
 *  discards the current input list up to the current token
 */
void (lst_discard)(int nested, int inc)
{
    lex_t *p;
    void *q;

    assert(ctx == &base || nested);
    assert(ctx->cur);                  /* implies assert(ctx->in) */

    p = ctx->in->next;
    while (p != ctx->cur) {
        q = p;
        p = p->next;
        if (((lex_t *)q)->id == LEX_MCR) {
            ((lex_t *)q)->next = q;
            ctx->out = lst_append(ctx->out, q);
        }
    }
    if (inc) {
        if (ctx->in == ctx->cur)
            ctx->in = NULL;
        else
            ctx->in->next = ctx->cur->next;
        if (ctx->cur->id == LEX_MCR) {
            ctx->cur->next = ctx->cur;
            ctx->out = lst_append(ctx->out, ctx->cur);
        }
        ctx->cur = NULL;
    } else
        ctx->in->next = ctx->cur;
}


/*
 *  retrieves a token from the input list
 */
static lex_t *nexti(void)
{
    assert(ctx == &base);

    while (1) {
        if (!ctx->cur) {
            if (ctx->in)
                ctx->cur = ctx->in->next;
            else
                ctx->in = ctx->cur = lex_next();
        } else {    /* implies assert(ctx->in) */
            if (ctx->cur == ctx->in)
                ctx->in = lst_append(ctx->in, lex_next());
            ctx->cur = ctx->cur->next;
        }
        if (ctx->cur->id == LEX_MCR) {
            if (ctx->cur->spell)
                ((ctx->cur->f.end)? mcr_edel: mcr_eadd)(ctx->cur->spell);
        } else
            break;
    }

    return ctx->cur;
}


/*
 *  retrieves a token from a token list
 */
static lex_t *nextl(void)
{
    assert(ctx != &base);
    assert(ctx->in);

    while (1) {
        if (ctx->cur == ctx->in)
            return &eoi;
        ctx->cur = (!ctx->cur)? ctx->in->next: ctx->cur->next;
        if (ctx->cur->id == LEX_MCR) {
            if (ctx->cur->spell)
                ((ctx->cur->f.end)? mcr_edel: mcr_eadd)(ctx->cur->spell);
        } else
            break;
    }

    return ctx->cur;
}


/*
 *  looks ahead the next token for function-like macros
 */
static lex_t *peeki(void)
{
    lex_t *p = ctx->cur;

    assert(ctx == &base);

    do {
        if (!p) {
            if (!ctx->in)
                ctx->in = lex_next();
            p = ctx->in;
        } else {
            if (p == ctx->in)
                ctx->in = lst_append(ctx->in, lex_next());
            p = p->next;
        }
    } while(p->id == LEX_SPACE || (p->id == LEX_NEWLINE && !lex_direc) || p->id == LEX_MCR);

    return p;
}


/*
 *  looks ahead the next token from a list for function-like macros
 */
static lex_t *peekl(void)
{
    lex_t *p = ctx->cur;

    assert(ctx != &base);
    assert(ctx->in);

    do {
        p = (!p)? ctx->in->next:
            (p == ctx->in)? &eoi: p->next;
    } while(p->id == LEX_SPACE || p->id == LEX_MCR);

    return p;
}


/*
 *  retrieves a token from the base output list
 */
lex_t *(lst_next)(void)
{
    lex_t *t;

    assert(base.out);

    if (base.out == base.out->next)    /* only one remained */
        proc_prep();
    t = base.out->next;
    base.out->next = base.out->next->next;

    return t;
}


/*
 *  copies a token
 */
lex_t *(lst_copy)(const lex_t *t, int mlev, arena_t *a)
{
    lex_t *p = ARENA_ALLOC(a, sizeof(*p));

    assert(t);

    memcpy(p, t, sizeof(*p));
    p->spell = (t->f.alloc)? hash_string(p->spell): p->spell;
    if (mlev == 0 && t->pos)
        p->pos = lmap_copy(t->pos, lmap_head, strg_perm);
    p->f.alloc = 0;
    p->next = p;

    return p;
}


/*
 *  copies a token list
 */
lex_t *(lst_copyl)(const lex_t *l, int mlev, arena_t *a)
{
    lex_t *p, *r;

    if (!l)
        return NULL;

    r = NULL;
    l = p = l->next;
    do {
        r = lst_append(r, lst_copy(p, mlev, a));
        p = p->next;
    } while(p != l);

    return r;
}


/*
 *  converts a token list to a null-terminated array
 */
lex_t **(lst_toarray)(lex_t *t, arena_t *a)
{
    lex_t **p;
    size_t i, n = 0;

    if (t) {
        lex_t *s = t;
        do {
            n++;
        } while((s = s->next) != t);
    }

    p = ARENA_ALLOC(a, (n+1)*sizeof(*p));
    for (i = 0; i < n; i++) {
        t = t->next;
        p[i] = t;
    }
    p[i] = NULL;

    return p;
}


/*
 *  constructs a token list from a string
 */
lex_t *(lst_run)(const char *s, const lmap_t *pos)
{
    lex_t *t;
    lex_t *l = NULL;

    assert(s);

    lex_backup();
    in_line = in_cp = s;
    in_limit = s + strlen(s);

    while ((t = lex_next())->id == LEX_SPACE)
        continue;
    if (t->id != LEX_EOI) {
        l = lst_append(l, t);
        while (1) {
            while ((t = lex_next())->id == LEX_SPACE)
                continue;
            if (t->id == LEX_EOI)
                break;
            l = lst_append(lst_append(l, lex_make(LEX_SPACE, " ", 0)), t);
        }
    }

    lex_restore();
    return l;
}


#ifndef NDEBUG
/*
 *  prints a token list for debugging
 */
void (lst_print)(lex_t *p, FILE *fp)
{
    lex_t *q;

    if (!p) {
        fputs("= input:\n", stderr);
        if (ctx->in)
            lst_print(ctx->in, fp);
        fputs("\n= output:\n", stderr);
        if (ctx->out)
            lst_print(ctx->out, fp);
        return;
    }

    q = p = p->next;
    do {
        fprintf(fp, "%c %p: ", (p == ctx->cur)? '*': '-', (void *)p);
        switch(p->id) {
            case -1:
                fprintf(fp, "[FREE] %p\n", p->spell);
                break;
            case LEX_MCR:
                fprintf(fp, "[%s] %s\n", (p->f.end)? "end": "start", (p->spell)? p->spell: "-");
                break;
            default:
                fprintf(fp, "%d(%s)%s\n", p->id, p->spell, (p->f.blue)? " !": "");
                break;
        }
        p = p->next;
    } while(p != q);
}
#endif    /* NDEBUG */

/* end of lst.c */
