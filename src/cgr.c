/*
 *  code generation rule
 */

#include <stddef.h>        /* NULL */
#include <cbl/assert.h>    /* assert */
#include <cbl/arena.h>     /* ARENA_CALLOC, ARENA_ALLOC */
#ifndef NDEBUG
#include <stdio.h>         /* FILE, fprintf, fputs, putc */
#endif    /* !NDEBUG */

#include "alist.h"
#include "common.h"
#include "op.h"
#include "strg.h"
#include "cgr.h"
#ifndef NDEBUG
#include "ir.h"
#endif    /* !NDEBUG */

/* generates hash key from op code */
#define h(op) ((((op) >> 8) + ((unsigned)(op) & 0xFF)) & (NELEM(rt)-1))


/* BURS rule table */
static struct entry {
    int op;                /* op code or non-terminal */
    alist_t *rule;         /* list of matched rules */
    struct entry *link;    /* hash chain */
} *rt[64];


/*
 *  finds a BURS rule from the hash table
 */
alist_t *(cgr_lookup)(int op)
{
    unsigned h;
    struct entry *p;

    h = h(op);
    for (p = rt[h]; p; p = p->link)
        if (op == p->op)
            return p->rule;

    return NULL;
}


/*
 *  adds a rule into the hash table
 */
cgr_t *(cgr_add)(cgr_t *r)
{
    int op;
    unsigned h;
    struct entry *p;

    assert(r);

    op = r->tree->op;
    h = h(op);
    for (p = rt[h]; p; p = p->link)
        if (op == p->op)
            break;

    if (!p) {
        p = ARENA_CALLOC(strg_perm, 1, sizeof(*p));
        p->op = op;
        p->link = rt[h];
        rt[h] = p;
    }
    p->rule = alist_append(p->rule, r, strg_perm);

    return r;
}


/*
 *  constructs a tree for BURS rules
 */
static cgr_tree_t *tree(int op, cgr_tree_t *l, cgr_tree_t *r)
{
    cgr_tree_t *p;

    p = ARENA_ALLOC(strg_perm, sizeof(*p));
    p->op = op;
    p->kid[0] = l;
    p->kid[1] = r;

    return p;
}


/*
 *  recursively generates a tree from a BURS rule
 */
static cgr_tree_t *treegen(const int **pp, unsigned char *pnnt)
{
    int op;
    const int *p;
    cgr_tree_t *kid[2] = { NULL, };

    assert(pp);
    assert(*pp);
    assert(pnnt);

    p = *pp;
    op = p[0];
    if (cgr_isnt(op))
        (*pnnt)++;
    switch(*++p) {
        case OP_1:
            *pp = ++p;
            kid[0] = treegen(pp, pnnt);
            break;
        case OP_2:
            *pp = ++p;
            kid[0] = treegen(pp, pnnt);
            kid[1] = treegen(pp, pnnt);
            break;
        default:
            *pp = p;
            break;
    }
    p = *pp;

    return tree(op, kid[0], kid[1]);
}


/*
 *  generates a tree from a BURS rule
 */
cgr_tree_t *(cgr_tree)(const int *p, unsigned char *pnnt)
{
    return treegen(&p, pnnt);
}


#ifndef NDEBUG
/*
 *  recursively prints a rule tree for debugging
 */
static void print(const cgr_tree_t *p, FILE *fp)
{
    assert(p);
    assert(fp);
    assert(ir_cur);

    if (cgr_isnt(p->op)) {    /* non-terminal */
        fprintf(fp, "%s", ir_cur->x.ntname(p->op));
        assert(!p->kid[0]);
        assert(!p->kid[1]);
    } else {
        fputs(op_name(p->op), fp);
        if (p->kid[0]) {
            putc('(', fp);
            print(p->kid[0], fp);
        }
        if (p->kid[1]) {
            fputs(", ", fp);
            print(p->kid[1], fp);
        }
        if (p->kid[0])
            putc(')', fp);
    }
}


/*
 *  prints a template escaping newlines
 */
void (cgr_tmpl)(const char *s, FILE *fp)
{
    int c;

    assert(s);
    assert(fp);

    while ((c = *s++) != '\0') {
        if (c == '\n')
            fputs("\\n", fp);
        else
            putc(c, fp);
    }
}


/*
 *  prints a rule for debugging
 */
void (cgr_print)(const cgr_t *p, FILE *fp)
{
    assert(p);
    assert(fp);
    assert(ir_cur);

    fprintf(fp, "%d: %s: ", p->rn, ir_cur->x.ntname(p->nt));
    print(p->tree, fp);
    fputs(" / [", fp);
    if (p->costf)
        fprintf(fp, "%p", (void *)p->costf);
    else
        fprintf(fp, "%d", p->cost);
    fputs(", \"", fp);
    cgr_tmpl(p->tmpl, fp);
    fputs("\"]\n", fp);
}
#endif    /* !NDEBUG */

/* end of cgr.c */
