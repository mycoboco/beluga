/*
 *  lexical linked-list for preprocessor
 */

#include <stdarg.h>        /* va_list, va_start, va_arg, va_end */
#include <stddef.h>        /* NULL */
#include <stdio.h>         /* FILE, fprintf, putc, fputs */
#include <cbl/arena.h>     /* arena_t, ARENA_ALLOC, ARENA_CALLOC */
#include <cbl/assert.h>    /* assert */

#include "ctx.h"
#include "lex.h"
#include "mcr.h"
#include "strg.h"
#include "lxl.h"


/*
 *  creates a new list
 */
lxl_t *(lxl_new)(arena_t *a)
{
    lxl_t *list;

    assert(a == strg_perm || a == strg_line);

    list = ARENA_ALLOC(a, sizeof(*list));
    list->head = list->tail = ARENA_CALLOC(a, 1, sizeof(*list->head));
    list->head->kind = LXL_KHEAD;
    list->head->strgno = (a == strg_perm)? -1: strg_no;

    return list;
}


/*
 *  appends a new node to a list
 */
lxl_node_t *(lxl_append)(lxl_t *list, int kind, ...)
{
    va_list ap;
    lxl_node_t *node;

    assert(list);

    va_start(ap, kind);
    /* CALLOC not used for performance */
    node = ARENA_ALLOC(strg_line, sizeof(*node));
    node->kind = kind;
    node->strgno = strg_no;
    switch(kind) {
        case LXL_KSTART:    /* ename, ppos */
        case LXL_KEND:
            node->u.e.n = va_arg(ap, const char *);
            node->u.e.ppos = va_arg(ap, const lex_pos_t *);
            break;
        case LXL_KTOK:    /* tok */
            node->u.t.tok = va_arg(ap, lex_t *);
            node->u.t.blue = 0;
            break;
        case LXL_KEOL:
            break;
        default:
            assert(!"invalid node kind -- should never reach here");
            break;
    }
    node->next = NULL;
    list->tail->next = node;
    list->tail = node;
    va_end(ap);

    return node;
}


/*
 *  duplicates a list
 */
lxl_t *(lxl_copy)(const lxl_t *list)
{
    lxl_t *new;
    lxl_node_t *p, **q;

    assert(list);

    new = ARENA_CALLOC(strg_line, 1, sizeof(*new));
    q = &new->head;
    for (p = list->head; p; p = p->next) {
        *q = ARENA_ALLOC(strg_line, sizeof(**q));
        assert(p->strgno == -1 || strg_no == p->strgno);
        **q = *p;
        if (!p->next)
            new->tail = *q;
        q = &(*q)->next;
    }

    return new;
}


/*
 *  converts an alist to a list;
 *  the blue flag is copied from a token
 */
lxl_t *(lxl_tolxl)(const alist_t *alist)
{
    alist_t *p;
    lxl_t *list = lxl_new(strg_line);

    if (alist) {
        p = alist->next;
        do {
            lxl_append(list, LXL_KTOK, (lex_t *)p->data);
            list->tail->u.t.blue = ((lex_t *)p->data)->f.blue;
            p = p->next;
        } while(p != alist->next);
    }

    return list;
}


/*
 *  inserts a new list after a node of another list
 */
lxl_node_t *(lxl_insert)(lxl_t *list, lxl_node_t *after, lxl_t *new)
{
    assert(list);
    assert(after);
    assert(new);

    if (!after->next)
        list->tail = new->tail;
    new->tail->next = after->next;
    after->next = new->head;

    return after;
}


/*
 *  retrieves a token from the current context;
 *  also sets the blue flag of token
 */
lex_t *(lxl_next)(void)
{
    static lex_t eoi = {
        LEX_EOI,
        ""
    };

    lxl_node_t *cur;

    assert(ctx_cur);
    assert(ctx_cur->cur);

    while (1) {
        if (!ctx_cur->cur->next)
            lxl_append(ctx_cur->list, LXL_KTOK, lex_nexttok());
        assert(ctx_cur->cur->next);
        ctx_cur->cur = ctx_cur->cur->next;
        cur = ctx_cur->cur;

        switch(cur->kind) {
            case LXL_KHEAD:
            case LXL_KTOKI:
                continue;
            case LXL_KSTART:
                if (ctx_cur->type == CTX_TNORM && cur->u.e.n)
                    mcr_eadd(cur->u.e.n);
                mcr_mpos = cur->u.e.ppos;
                continue;
            case LXL_KEND:
                if (cur->u.e.n) {
                    if (ctx_cur->type == CTX_TNORM)
                        mcr_edel(cur->u.e.n);
                    else
                        mcr_emeet(cur->u.e.n);
                }
                mcr_mpos = cur->u.e.ppos;
                continue;
            case LXL_KEOL:
                assert(!ctx_isbase());
                return &eoi;
            case LXL_KTOK:
                if (ctx_cur->type == CTX_TIGNORE)
                    cur->kind = LXL_KTOKI;
                cur->u.t.tok->f.blue = cur->u.t.blue;
                return cur->u.t.tok;
            default:
                assert(!"invalid node kind -- should never reach here");
                break;
        }
    }

    /* assert(!"impossible control flow -- should never reach here");
       return NULL; */
}


/*
 *  prints a list for debugging
 */
void (lxl_print)(const lxl_t *list, const lxl_node_t *cur, FILE *fp)
{
    lxl_node_t *p;

    assert(list);
    assert(fp);

    for (p = list->head; p; p = p->next) {
        fprintf(fp, "%c%c%p(%d): ", (cur && p == cur)? '*': ' ',
                                    (p->kind == LXL_KTOKI)? '-': ' ',
                                    (void *)p, p->strgno);
        switch(p->kind) {
            case LXL_KHEAD:
                fputs("[head]\n", fp);
                break;
            case LXL_KSTART:
            case LXL_KEND:
                fprintf(fp, "[%s]", (p->kind == LXL_KSTART)? "start": "end");
                if (p->u.e.n)
                    fprintf(fp, " %s", p->u.e.n);
                if (p->u.e.ppos)
                    fprintf(fp, " @%s", lex_outpos(p->u.e.ppos));
                putc('\n', fp);
                break;
            case LXL_KTOK:
            case LXL_KTOKI:
                if (p->u.t.tok->id == LEX_NEWLINE)
                    fprintf(fp, "%d(%s) @%s\n", p->u.t.tok->id, "\\n",
                            lex_outpos((lex_pos_t *)p->u.t.tok->rep));
                else
                    fprintf(fp, "%d(%s)%s\n", p->u.t.tok->id, p->u.t.tok->rep,
                                              (p->u.t.blue)? " !": "");
                break;
            case LXL_KEOL:
                fputs("[eoi]\n", fp);
                break;
            default:
                assert(!"invalid node kind -- should never reach here");
                break;
        }
    }
}

/* end of lxl.c */
