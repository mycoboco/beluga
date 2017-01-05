/*
 *  code generator
 */

#include <ctype.h>         /* isdigit */
#include <stdarg.h>        /* va_list, va_start, va_end */
#include <stddef.h>        /* size_t, NULL */
#include <stdio.h>         /* vsprintf */
#include <string.h>        /* strchr */
#include <cbl/arena.h>     /* ARENA_ALLOC */
#include <cbl/assert.h>    /* assert */
#include <cdsl/hash.h>     /* hash_int, hash_string */
#ifndef NDEBUG
#include <stdio.h>         /* FILE, fprintf, fputs, putc, stderr */
#endif    /* !NDEBUG */

#include "alist.h"
#include "cgr.h"
#include "common.h"
#include "dag.h"
#include "ir.h"
#include "lex.h"
#include "op.h"
#include "reg.h"
#include "strg.h"
#include "sym.h"
#include "gen.h"

#define relink(a, b) ((b)->x.prev = (a), (a)->x.next = (b))    /* links two lists */

/* checks if dag reads register */
#define READREG(p) (op_generic((p)->op) == OP_INDIR && (p)->kid[0]->op == OP_VREGP)


int gen_off, gen_maxoff;      /* offset and max offset for locals */
int gen_aoff, gen_maxaoff;    /* offset and max offset for arguments */
int gen_frame;                /* frame size */


/* internal functions referenced forwardly */
static void chain(int, dag_node_t *, int);
static void prereduce(dag_node_t **, int);
#ifndef NDEBUG
static void dumplabel(const dag_node_t *, FILE *);
static void dumpcover(dag_node_t *, FILE *);
#endif    /* !NDEBUG */


/*
 *  prepares rewriting;
 *  - remembers the argument offset for CALL;
 *  ASSUMPTION: the number of arguments can be fixed at compile time
 */
static void prerewrite(dag_node_t *p)
{
    assert(p);

    switch(op_generic(p->op)) {
        case OP_ASGN:
            prerewrite(p->kid[1]);
            break;
        case OP_CALL:
            p->sym[0] = sym_findint(gen_aoff);
            if (gen_aoff > gen_maxaoff)
                gen_maxaoff = gen_aoff;
            gen_aoff = 0;
            break;
    }
}


/*
 *  prepares to label dag nodes
 */
static void prelabel(dag_node_t *p)
{
    assert(ir_cur);

    if (!p)
        return;

    prelabel(p->kid[0]);
    prelabel(p->kid[1]);

    p->sym[REG_RX] = NULL;
    switch(op_generic(p->op)) {
        case OP_ADDRF:
        case OP_ADDRL:
            if (p->sym[0]->sclass == LEX_REGISTER)
                p->op = OP_VREGP;
            break;
        case OP_INDIR:
            if (p->kid[0]->op == OP_VREGP)
                reg_set(p, p->kid[0]->sym[0]);
            break;
        case OP_ASGN:
            if (p->kid[0]->op == OP_VREGP) {
                DEBUG(fprintf(stderr, "- cse: %p\n", p->kid[0]->sym[0]->u.t.cse));
                reg_target(p, 1, p->kid[0]->sym[0]);
            }
            break;
    }
    ir_cur->x.target(p);
}


/*
 *  checks if a BURS rule matches a dag
 */
static int match(dag_node_t *p, cgr_tree_t *q)
{
    if ((void *)p == q) {
        assert(!p);
        return 0;
    }

    if (cgr_isnt(q->op))
        return (!p->x.cost)? CGR_CSTMAX: p->x.cost[cgr_ntidx(q->op)];
    else if (p->op == q->op)
        return match(p->kid[0], q->kid[0]) + match(p->kid[1], q->kid[1]);

    return CGR_CSTMAX;
}


/*
 *  sets a cost and follows chain rules
 */
static void setcost(cgr_t *x, dag_node_t *p, int w)
{
    int i;
    int cost;

    assert(x);
    assert(x->rn >= 0 && x->nt > 0);
    assert(p);
    assert(ir_cur);

    if (w >= CGR_CSTMAX)    /* avoids costf() for unmatched dags; see memop() */
        return;
    cost = ((x->costf)? x->costf(p): x->cost) + w;
    if (cost >= CGR_CSTMAX)
        return;
    if (!p->x.rn) {    /* allocates only when necessary */
        p->x.rn = ARENA_ALLOC(strg_perm, ir_cur->x.nnt * sizeof(*p->x.rn));
        p->x.cost = ARENA_ALLOC(strg_perm, ir_cur->x.nnt * sizeof(*p->x.cost));
        p->x.rn[0] = 0;
        for (i = 0; i < ir_cur->x.nnt ; i++)
            p->x.cost[i] = CGR_CSTMAX;
    } else if (cost >= p->x.cost[cgr_ntidx(x->nt)])
        return;
    p->x.cost[cgr_ntidx(x->nt)] = cost;
    p->x.rn[cgr_ntidx(x->nt)] = x->rn;
    chain(x->nt, p, cost);
}


/*
 *  follows chain rules
 */
static void chain(int nt, dag_node_t *p, int w)
{
    size_t n;
    alist_t *ci, *c = cgr_lookup(nt);

    ALIST_FOREACH(n, ci, c) {
        setcost(ci->data, p, w);
    }
}


/*
 *  BURS labeller
 */
static void label(dag_node_t *p)
{
    size_t n;
    alist_t *r, *ri;

    if (!p)
        return;
    label(p->kid[0]);
    label(p->kid[1]);

    assert(!p->x.rn);
    assert(!p->x.cost);

    r = cgr_lookup(p->op);
    ALIST_FOREACH(n, ri, r) {
        setcost(ri->data, p, match(p, ((cgr_t *)ri->data)->tree));
    }

    /* corresponds to reuse() */
    if (op_generic(p->op) == OP_INDIR && p->kid[0]->op == OP_VREGP) {
        sym_t *s = p->sym[REG_RX];
        if (s && s->u.t.cse && !s->u.t.cse->usecse) {
            int i;
            dag_node_t *q = s->u.t.cse;
            assert(p->x.cost && q->x.cost);
            for (i = 0; i < ir_cur->x.nnt; i++)
                if (q->x.cost[i] == 0) {
                    p->x.cost[i] = 0;
                    p->x.rn[i] = q->x.rn[i];
                }
        }
    }
}


/*
 *  finds non-terminals in children of a dag
 */
static dag_node_t **findnt(dag_node_t **pp, const cgr_tree_t *r, dag_node_t *ppv[])
{
    assert(pp);
    assert(ppv);

    if (!r)
        return ppv;

    if (cgr_isnt(r->op)) {
        prereduce(pp, cgr_ntidx(r->op));
        *ppv = *pp;
        return ppv + 1;
    } else {
        dag_node_t *p = *pp;
        assert(p);
        return findnt(&p->kid[1], r->kid[1], findnt(&p->kid[0], r->kid[0], ppv));
    }
}


/*
 *  returns a copy of the original node for a cse if bonus match
 */
static dag_node_t *reuse(dag_node_t *p, int idx)
{
    sym_t *s = p->sym[REG_RX];

    assert(p);
    assert(p->x.cost);    /* failure indicates no match from labeller */

    if (op_generic(p->op) == OP_INDIR && p->kid[0]->op == OP_VREGP && s && s->u.t.cse &&
        !s->u.t.cse->usecse) {
        dag_node_t *r = s->u.t.cse;
        if (r->x.cost && r->x.cost[idx] == 0)
            return dag_copy(r);
    }

    return p;
}


/*
 *  prepares to reduce dag nodes
 */
static void prereduce(dag_node_t **pp, int idx)
{
    dag_node_t *p;
    const cgr_t *r;

    assert(pp);
    assert(ir_cur && idx > 0 && idx < ir_cur->x.nnt);

    p = *pp = reuse(*pp, idx);
    assert(p);
    assert(p->x.rn);    /* failure indicates no match from labeller */

    r = &ir_cur->x.rule[p->x.rn[idx]];
    assert(r->rn >= 0 && r->nt > 0);

    if (cgr_isnt(r->tree->op))    /* chain rule */
        prereduce(pp, cgr_ntidx(r->tree->op));
    else if (r->nnt > 0) {
        dag_node_t **ppv;
        ppv = p->x.vr = ARENA_ALLOC(strg_perm, (r->nnt+1)*sizeof(*p->x.vr));
        ppv[r->nnt] = NULL;
        findnt(&p->kid[1], r->tree->kid[1], findnt(&p->kid[0], r->tree->kid[0], ppv));
    }
}


/*
 *  reduces dag nodes
 */
static void reduce(dag_node_t *p, int idx, int lev,
                   void (*apply)(dag_node_t *, const cgr_t *, int, void *), void *cl)
{
    const cgr_t *r;

    assert(p);
    assert(p->x.rn);
    assert(ir_cur && idx > 0 && idx < ir_cur->x.nnt);
    assert(apply);

    r = &ir_cur->x.rule[p->x.rn[idx]];
    assert(r->rn >= 0 && r->nt > 0);
    apply(p, r, lev, cl);

    if (cgr_isnt(r->tree->op))    /* chain rule */
        reduce(p, cgr_ntidx(r->tree->op), lev+1, apply, cl);
    else if (p->x.vr) {
        const int *pt;
        dag_node_t **pp = p->x.vr;
        for (pt = r->ot; *pt > 0; pt++)
            if (cgr_isnt(*pt))
                reduce(*pp++, cgr_ntidx(*pt), lev+1, apply, cl);
    }
}


/*
 *  marks a dag node if it contains an instruction
 */
static void markinst(dag_node_t *p, const cgr_t *r, int lev, void *dummy)
{
    assert(p);
    assert(r);

    UNUSED(lev);
    UNUSED(dummy);

    if (r->isinst) {
        sym_t *s = p->sym[REG_RX];
        assert(!p->x.inst || p->x.inst == cgr_ntidx(r->nt));
        p->x.inst = cgr_ntidx(r->nt);
        if (s && s->u.t.cse) {
            s->x.usecnt++;
            DEBUG(fprintf(stderr, "= cse(%s) used (%d)\n", s->name, s->x.usecnt));
        }
    }
}


/*
 *  rewrites a dag by labelling and reducing
 */
void (gen_rewrite)(dag_node_t *p)
{
    assert(p);
    assert(!p->x.inst);

    prelabel(p);
    label(p);
    DEBUG(dumplabel(p, stderr));

    prereduce(&p, 1);
    DEBUG(dumpcover(p, stderr));
    reduce(p, 1, 0, markinst, NULL);
}


/*
 *  prunes a dag for register allocation
 */
dag_node_t **(gen_prune)(dag_node_t *p, dag_node_t *pp[], dag_node_t **b)
{
    int i;

    assert(pp);
    assert(b);

    if (!p)
        return pp;

    for (i = 0; i < NELEM(p->x.kid); i++)
        p->x.kid[i] = NULL;
    if (!p->x.inst)
        return gen_prune(p->kid[1], gen_prune(p->kid[0], pp, b), b);
    else {
        sym_t *s = p->sym[REG_RX];
        if (s && s->u.t.cse && s->x.usecnt < 2) {
            p->x.inst = 0;
            DEBUG(fprintf(stderr, "= cse(%s) removed because not used\n", s->name));
            return gen_prune(p->kid[1], gen_prune(p->kid[0], pp, b), b);
        } else {
            dag_node_t **n = &p->x.kid[NELEM(p->x.kid)];
            gen_prune(p->kid[1], gen_prune(p->kid[0], &p->x.kid[0], n), n);
            assert(pp < b);
            *pp = p;
            return pp + 1;
        }
    }
}


/*
 *  constructs a list of instructions from the forest
 */
void (gen_linearize)(dag_node_t *p, dag_node_t *n)
{
    int i;

    assert(p);
    assert(n);

    FORXKIDS(p, 0) {
        gen_linearize(p->x.kid[i], n);
    }
    relink(n->x.prev, p);
    relink(p, n);

    DEBUG(fprintf(stderr, "= listing %p\n", p));
}


/*
 *  allocates wildcards
 */
void (gen_wildcard)(dag_node_t *p)
{
    assert(ir_cur);

    for (; p; p = p->x.next)
        if (!p->sym[REG_RX] && REG_NEED(p))
            reg_set(p, ir_cur->x.rmapw(p->op));
}


/*
 *  processes a dag;
 *  should be exported via ir
 */
dag_node_t *(gen_code)(dag_node_t *forest)
{
    int i;
    sym_t *s;
    dag_node_t *p, *dummy, sentinel;

    assert(forest);
    assert(ir_cur);

    DEBUG(fputs(">> forest starts\n", stderr));
    for (p = forest; p; p = p->link) {
        assert(p->count == 0);
        prerewrite(p);
        ir_cur->x.prerewrite(p);
        gen_rewrite(p);
        p->x.f.listed = 1;
    }

    for (p = forest; p; p = p->link)
        gen_prune(p, &dummy, &dummy+1);

    relink(&sentinel, &sentinel);
    for (p = forest; p; p = p->link)
        gen_linearize(p, &sentinel);
    forest = sentinel.x.next;
    assert(forest);
    sentinel.x.next->x.prev = NULL;
    sentinel.x.prev->x.next = NULL;

    gen_wildcard(forest);

    for (p = forest; p; p = p->x.next)
        FORXKIDS(p, 0) {
            s = p->x.kid[i]->sym[REG_RX];
            assert(s);
            if (s->u.t.cse) {
                p->x.kid[i]->x.prevuse = s->x.lastuse;
                s->x.lastuse = p->x.kid[i];
            }
        }

    for (p = forest; p; p = p->x.next)
        if (p->x.f.copy && p->x.kid[0]->sym[REG_RX]->u.t.cse) {
            sym_t *tmp = p->x.kid[0]->sym[REG_RX],
                  *dst = p->sym[REG_RX];
            dag_node_t *q;
            int flag = 0;

            assert(!dst->x.wildcard && tmp->x.lastuse);
            for (q = tmp->u.t.cse; q; q = q->x.next) {
                FORXKIDS(q, 0) {
                    if (q->x.kid[i] == tmp->x.lastuse)
                        flag = 1;
                }
                if (p != q &&
                    (dst == q->sym[REG_RX] ||
                     (dst->x.regnode && REG_FORRVAR(dst, q->sym[REG_RX]))) &&
                    !READREG(q)) {
                    assert(!dst->u.t.cse);
                    break;
                }
                if (dst->x.regnode && REG_ISRVAR(dst) &&
                    (q->op == OP_LABELV || q->op == OP_JMPV || op_generic(q->op) == OP_RET ||
                     op_generic(q->op) == OP_EQ || op_generic(q->op) == OP_NE ||
                     op_generic(q->op) == OP_LE || op_generic(q->op) == OP_LT ||
                     op_generic(q->op) == OP_GE || op_generic(q->op) == OP_GT) && q->x.next)
                    break;
            }
            if (!q || flag) {
                if (!(dst->x.regnode && REG_ISRVAR(dst)))
                    dst->x.lastuse = tmp->x.lastuse;
                for (q = tmp->x.lastuse; q; q = q->x.prevuse) {
                    reg_set(q, dst);
                    if (!dst->u.t.cse && (q->x.prevuse || (dst->x.regnode && REG_ISRVAR(dst))))
                        q->x.f.registered = 1;
                }
                if (dst->u.t.cse) {
                    dst->x.lastuse = NULL;
                    for (q = tmp->u.t.cse; q; q = q->x.next)
                        FORXKIDS(q, 0) {
                            s = q->x.kid[i]->sym[REG_RX];
                            assert(s);
                            if (s == dst) {
                                q->x.kid[i]->x.prevuse = s->x.lastuse;
                                s->x.lastuse = q->x.kid[i];
                            }
                        }
                }
            }
        }

    for (p = forest; p; p = p->x.next) {
        const cgr_t *r;
        assert(p->x.inst > 0 && p->x.inst < ir_cur->x.nnt);
        assert(p->x.rn);
        r = &ir_cur->x.rule[p->x.rn[p->x.inst]];
        assert(r->rn >= 0 && r->nt > 0);
        s = p->sym[REG_RX];
        if (NELEM(p->x.kid) > 1 && r->tmpl[0] == '?' && p->x.kid[1] &&
            s && !s->x.wildcard && !s->u.t.cse && !REG_ISRVAR(s))    /* targetted */
            reg_pmask(p);
    }
    for (p = forest; p; p = p->x.next)
        reg_alloc(p);

    DEBUG(fputs("<< forest ends\n", stderr));
    return forest;
}


/*
 *  starts a block;
 *  should be exported via ir
 */
void (gen_blkbeg)(gen_env_t *env)
{
    int i;

    assert(env);

    env->offset = gen_off;
    for (i = 0; i < REG_SMAX; i++)
        env->fmask[i] = reg_mbackup(reg_fmask[i]);
}


/*
 *  ends a block;
 *  should be exported via ir;
 *  ASSUMPTION: frame size can be fixed at compile time
 */
void (gen_blkend)(const gen_env_t *env)
{
    int i;

    assert(env);

    if (gen_off > gen_maxoff)
        gen_maxoff = gen_off;
    gen_off = env->offset;
    for (i = 0; i < REG_SMAX; i++)
        reg_mrestore(reg_fmask[i], env->fmask[i]);
}


/*
 *  calculates the local offset
 */
void (gen_auto)(sym_t *p, int a)
{
    assert(p);
    assert(p->sclass == LEX_AUTO);

    gen_off = ROUNDUP(gen_off + p->type->size, (p->type->align < a)? a: p->type->align);
    p->x.offset = -gen_off;
    p->x.name = hash_int(-gen_off);
}


/*
 *  calculates the argument offset
 */
int (gen_arg)(int size, int align)
{
    int aoff = gen_aoff;

    assert(size > 0);
    assert(align > 0);

    gen_aoff = ROUNDUP(aoff + size, align);

    return aoff;
}


#define samename(s, t) ((s)->x.name == (t)->x.name)

/*
 *  checks if a reg-to-reg copy is meaningless
 */
static int moveself(const dag_node_t *p)
{
    assert(p);

    return (p->x.f.copy && (assert(p->sym[REG_RX] && p->x.kid[0] && p->x.kid[0]->sym[REG_RX]),
                            samename(p->sym[REG_RX], p->x.kid[0]->sym[REG_RX])));
}


/*
 *  optimizes a reg-to-reg copy to a cse temporary
 */
static int equate(dag_node_t *p)
{
    int n = 0;
    sym_t *src, *dst;
    dag_node_t *q;

    assert(p);
    assert(p->x.kid[0]);
    dst = p->sym[REG_RX];
    src = p->x.kid[0]->sym[REG_RX];
    assert(dst && src);

    DEBUG(fprintf(stderr, "= equate %p with src(%s) and dst(%s)\n",
                  (void *)p, src->x.name, dst->x.name));
    for (q = p->x.next; q; q = q->x.next) {
        if (q->x.f.copy && (assert(q->sym[REG_RX]), samename(q->sym[REG_RX], src)) &&
            (assert(q->x.kid[0] && q->x.kid[0]->sym[REG_RX]),
             samename(q->x.kid[0]->sym[REG_RX], dst))) {
            DEBUG(fprintf(stderr, " - equate copyback (%p)->x.kid[0] from %s to %s\n",
                          (void *)q, dst->x.name, src->x.name));
            reg_set(q->x.kid[0], src);
        } else if (q->sym[REG_RX] && reg_shares(q->sym[REG_RX], src) && !moveself(q) &&
                   !READREG(q)) {    /* src changes */
            dag_node_t *r = q;
            while ((r = r->x.next) != NULL)
                if (r->sym[REG_RX] && samename(r->sym[REG_RX], dst) && READREG(r))
                    return 0;    /* dst read after src changed */
        } else if (q->x.f.spill)
            return 0;
        else if (q->op == OP_LABELV && q->x.next)
            return 0;
        else if (q->sym[REG_RX] && samename(q->sym[REG_RX], dst)) {
            if (READREG(q)) {
                n++;    /* x.lastuse not reliable after spill */
                DEBUG(fprintf(stderr, " - n = %d at %p\n", n, (void *)q));
            } else
                break;
        }
    }

    if (n > 0) {
        DEBUG(fprintf(stderr, " - equate starts at %p\n", (void *)q));
        for (q = p->x.next; q; q = q->x.next)
            if (q->sym[REG_RX] && samename(q->sym[REG_RX], dst)) {
                DEBUG(fprintf(stderr, " - equate %p from %s to %s\n",
                            (void *)q, dst->x.name, src->x.name));
                reg_set(q, src);
                if (--n == 0)
                    break;
            }
    }

    return 1;
}

#undef samename


/*
 *  emits assembly code for an instruction;
 *  cannot use reduce() because emitasm() works for one cover
 */
static void emitasm(dag_node_t *p, int idx)
{
    const cgr_t *r;
    const char *tmpl;
    FILE *out;
    const char *abc = "abcdefghijklmnopqrstuvwxyz", *q;

    assert(p);
    assert(p->x.rn);
    assert(ir_cur && idx > 0 && idx < ir_cur->x.nnt);

    out = ir_cur->out;
    assert(out);

    r = &ir_cur->x.rule[p->x.rn[idx]];
    assert(r->rn >= 0 && r->nt > 0);

    tmpl = r->tmpl;
    DEBUG(fprintf(stderr, "= emit (%p) with ", (void *)p));
    DEBUG(cgr_tmpl(tmpl, stderr));
    DEBUG(putc('\n', stderr));
    if (r->isinst && p->x.f.emitted) {
        assert(p->sym[REG_RX]);
        fputs(p->sym[REG_RX]->x.name, out);
    } else if (*tmpl == '#')
        ir_cur->x.emit(p);
    else {
        if (*tmpl == '?') {
            tmpl++;    /* skips ? */
            assert(p->x.kid[0] && p->x.kid[0]->sym[REG_RX]);
            if (p->sym[REG_RX] == p->x.kid[0]->sym[REG_RX])
                while (*tmpl++ != '\n')
                    continue;
        }
        for (; *tmpl; tmpl++)
            if (*tmpl != ir_cur->x.fmt)
                putc(*tmpl, out);
            else if (*++tmpl == 'F')    /* skips %; %F */
                fprintf(out, "%d", gen_frame);
            else if (*tmpl == 'R')    /* %R */
                fprintf(out, "%s", p->sym[REG_RX]->x.name);
            else if (isdigit(*(unsigned char *)tmpl)) {    /* %[0-9] */
                if (cgr_isnt(r->tree->op)) {    /* chain rule */
                    assert(*tmpl == '0');
                    emitasm(p, cgr_ntidx(r->tree->op));
                } else {
                    int n = 0;
                    const int *pt;
                    dag_node_t **pp = p->x.vr;
                    assert(pp);
                    for (pt = r->ot; *pt > 0; pt++)
                        if (cgr_isnt(*pt)) {
                            if (n == *tmpl-'0') {
                                emitasm(*pp, cgr_ntidx(*pt));
                                break;
                            }
                            n++, pp++;
                        }
                    assert(*pt > 0);
                }
            } else if ((q = strchr(abc, *tmpl)) != NULL) {    /* %[a-z] */
                assert(p->sym[q-abc]);
                fprintf(out, "%s", p->sym[q-abc]->x.name);
            } else
               putc(*tmpl, out);
    }
}


/*
 *  triggers an emitter following a linearized dag
 */
void (gen_emit)(dag_node_t *p)
{
    for (; p; p = p->x.next) {
        assert(p->x.f.registered);
        if (!moveself(p) && !(p->x.f.equatable && equate(p)))
            emitasm(p, p->x.inst);
        p->x.f.emitted = 1;
    }
}


/*
 *  provides a formatted string function to the back-end
 */
const char *(gen_sfmt)(size_t n, const char *fmt, ...)
{
    static size_t bn;
    static char *pbuf;

    va_list ap;

    assert(n > 0);
    assert(fmt);

    if (bn < n+1) {
        bn = n + 1;
        pbuf = ARENA_ALLOC(strg_perm, bn);
    }
    va_start(ap, fmt);
    vsprintf(pbuf, fmt, ap);
    va_end(ap);

    return hash_string(pbuf);
}


/*
 *  common cost function: move
 */
int (gen_move)(dag_node_t *p)
{
    assert(p);
    p->x.f.copy = 1;
    return 1;
}


/*
 *  common cost function: notarget
 */
int (gen_notarget)(dag_node_t *p)
{
    assert(p);
    return (!p->sym[REG_RX])? 0: CGR_CSTMAX;
}


#ifndef NDEBUG
/*
 *  recusively prints a labelled dag for debugging
 */
static void dump(const dag_node_t *p, FILE *fp, int lev)
{
    int i;

    assert(fp);
    assert(ir_cur);

    if (!p)
        return;

    for (i = 0; i < lev; i++)
        putc(' ', fp);
    fprintf(fp, "[%p] %s\n", (void *)p, op_name(p->op));

    if (p->x.rn)
        for (i = 0; i < ir_cur->x.nnt; i++) {
            if (p->x.cost[i] < CGR_CSTMAX) {
                int j;
                for (j = 0; j < lev; j++)
                    putc(' ', fp);
                fprintf(fp, "(%s) ", ir_cur->x.ntname(cgr_idxnt(i)));
                cgr_print(&ir_cur->x.rule[p->x.rn[i]], fp);
            }
        }

    dump(p->kid[0], fp, lev+1);
    dump(p->kid[1], fp, lev+1);
}


/*
 *  prints a labelled dag for debugging
 */
static void dumplabel(const dag_node_t *p, FILE *fp)
{
    dump(p, fp, 0);
    putc('\n', fp);
}


/*
 *  provides a callback to a reducer to print covers
 */
static void cover(dag_node_t *p, const cgr_t *q, int lev, void *fp)
{
    int i;

    UNUSED(p);
    assert(fp);

    for (i = 0; i < lev; i++)
        putc(' ', fp);
    cgr_print(q, fp);
}


/*
 *  prints covers for debugging
 */
static void dumpcover(dag_node_t *p, FILE *fp)
{
    assert(fp);

    reduce(p, 1, 0, cover, fp);
    putc('\n', fp);
}
#endif    /* !NDEBUG */

/* end of gen.c */
