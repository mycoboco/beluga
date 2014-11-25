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


#define newnode() (node = ARENA_ALLOC(strg_line, sizeof(*node)),    \
                   node->kind = kind, node->strgno = strg_no)

/*
 *  appends a new node to a list;
 *  CALLOC not used for performance
 */
lxl_node_t *(lxl_append)(lxl_t *list, int kind, ...)
{
    va_list ap;
    lxl_node_t *node;

    assert(list);

    va_start(ap, kind);
    switch(kind) {
        case LXL_KEOL:    /* always new node */
            newnode();
            node->e = ARENA_ALLOC(strg_line, sizeof(*node->e));
            break;
        case LXL_KSTART:    /* ename, ppos; always new node */
        case LXL_KEND:
            newnode();
            node->e = ARENA_ALLOC(strg_line, sizeof(*node->e));
            node->e->n = va_arg(ap, const char *);
            node->e->ppos = va_arg(ap, const lex_pos_t *);
            break;
        case LXL_KTOK:    /* tok */
            if (list->tail->kind <= LXL_KEND) {    /* can share */
                node = list->tail;
                node->kind |= kind;
                node->t.tok = va_arg(ap, lex_t *);
                node->t.blue = 0;
                goto ret;
            }
            newnode();
            node->t.tok = va_arg(ap, lex_t *);
            node->t.blue = 0;
            break;
        default:
            assert(!"invalid node kind -- should never reach here");
            break;
    }
    node->next = NULL;
    list->tail->next = node;
    list->tail = node;

    ret:
        va_end(ap);

        return node;
}

#undef newnode


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
            list->tail->t.blue = ((lex_t *)p->data)->f.blue;
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

    int kind;
    lxl_node_t *cur;

    assert(ctx_cur);
    assert(ctx_cur->cur);

    while (1) {
        if (!ctx_cur->cur->next &&
            (lxl_append(ctx_cur->list, LXL_KTOK, lex_nexttok()), !ctx_cur->cur->next)) {
            kind = lxl_post(cur);
        } else {
            ctx_cur->cur = ctx_cur->cur->next;
            cur = ctx_cur->cur;
            kind = lxl_kind(cur);
        }

        switch(kind) {
            case LXL_KHEAD:
            case LXL_KTOKI:
                continue;
            case LXL_KEOL:
                assert(!ctx_isbase());
                return &eoi;
            case LXL_KSTART:
                if (ctx_cur->type == CTX_TNORM && cur->e->n)
                    mcr_eadd(cur->e->n);
                mcr_mpos = cur->e->ppos;
                if (!lxl_isshared(cur) || lxl_post(cur) == LXL_KTOKI)
                    continue;
                goto tok;
            case LXL_KEND:
                if (cur->e->n) {
                    if (ctx_cur->type == CTX_TNORM)
                        mcr_edel(cur->e->n);
                    else
                        mcr_emeet(cur->e->n);
                }
                mcr_mpos = cur->e->ppos;
                if (!lxl_isshared(cur) || lxl_post(cur) == LXL_KTOKI)
                    continue;
                /* no break */
            tok:
                assert(lxl_post(cur) == LXL_KTOK);
            case LXL_KTOK:
                if (ctx_cur->type == CTX_TIGNORE)
                    LXL_IGNORE(cur);
                cur->t.tok->f.blue = cur->t.blue;
                return cur->t.tok;
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
    int kind;
    lxl_node_t *p;

    assert(list);
    assert(fp);
    assert(list->head);

    p = list->head;
    kind = lxl_kind(p);
    while (1) {
        fprintf(fp, "%c%c%p(%d): ", (p == cur)? '*': ' ',
                                    (kind == LXL_KTOKI)? '-': ' ',
                                    (void *)p, p->strgno);
        switch(kind) {
            case LXL_KHEAD:
                fputs("[head]\n", fp);
                break;
            case LXL_KEOL:
                fputs("[eoi]\n", fp);
                break;
            case LXL_KSTART:
            case LXL_KEND:
                fprintf(fp, "[%s]", (kind == LXL_KSTART)? "start": "end");
                if (p->e->n)
                    fprintf(fp, " %s", p->e->n);
                if (p->e->ppos)
                    fprintf(fp, " @%s", lex_outpos(p->e->ppos));
                putc('\n', fp);
                if (kind == p->kind)
                    break;
                else {
                    kind = lxl_post(p);
                    continue;
                }
            case LXL_KTOK:
            case LXL_KTOKI:
                if (p->t.tok->id == LEX_NEWLINE)
                    fprintf(fp, "%d(%s) @%s\n", p->t.tok->id, "\\n",
                            lex_outpos((lex_pos_t *)p->t.tok->rep));
                else
                    fprintf(fp, "%d(%s)%s\n", p->t.tok->id, p->t.tok->rep,
                                              (p->t.blue)? " !": "");
                break;
            default:
                assert(!"invalid node kind -- should never reach here");
                break;
        }
        p = p->next;
        if (!p)
            break;
        kind = lxl_kind(p);
    }
}

/* end of lxl.c */
