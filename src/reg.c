/*
 *  register
 */

#include <stdarg.h>        /* va_list, va_start, va_arg, va_end */
#include <stddef.h>        /* NULL */
#include <string.h>        /* strlen, memset, memcpy */
#include <cbl/arena.h>     /* arena_t, ARENA_CALLOC, ARENA_ALLOC */
#include <cbl/assert.h>    /* assert */
#include <cdsl/hash.h>     /* hash_string */
#ifndef NDEBUG
#include <stdio.h>         /* FILE, stderr, fprintf, vfprintf, putc */
#endif    /* !NDEBUG */

#include "common.h"
#include "dag.h"
#include "gen.h"
#include "ir.h"
#include "lex.h"
#include "op.h"
#include "reg.h"
#include "strg.h"
#include "sym.h"
#include "ty.h"
#include "reg.h"

#define BPW (8 * sizeof(unsigned long))    /* # of bits per word */

#define nword(n) (((n)+BPW-1) / BPW)    /* # of words in bit-vector of length n */
#define nbyte(n) (((n)+8-1) / 8)        /* # of bytes in bit-vector of length n */

#define byte(s) ((unsigned char *)(s))    /* accesses bit-vector via byte */

/* extracts bit from bit-vector */
#define BIT(s, n) (byte(s)[(n)/8] >> ((n)%8) & 1)

/* checks if node reads register */
#define READ(p) (op_generic((p)->op) == OP_INDIR && (p)->kid[0]->op == OP_VREGP)

/* finds the first use of register */
#define FIRSTUSE(s, f)                          \
    do {                                        \
        (f) = (s)->x.lastuse, assert(f);        \
        while ((f)->x.prevuse)                  \
            assert((f)->sym[REG_RX] == (s)),    \
            (f) = (f)->x.prevuse;               \
    } while(0)


reg_mask_t *reg_fmask[REG_SMAX],    /* free register mask */
           *reg_umask[REG_SMAX],    /* used register mask */
           *reg_tmask[REG_SMAX],    /* temporary register mask */
           *reg_vmask[REG_SMAX];    /* register variable mask */


#ifndef NDEBUG
/* internal functions referenced forwardly */
static void printbv(reg_mask_t *[], FILE *);
static void printreg(FILE *, const char *, ...);
#endif    /* !NDEBUG */


/*
 *  constructs a new bit-vector for registers
 */
static reg_mask_t *newbitv(arena_t *a)
{
    assert(a);
    assert(ir_cur && ir_cur->x.nreg > 0);

    return ARENA_CALLOC(a, 1, nword(ir_cur->x.nreg)*sizeof(reg_mask_t));
}


/*
 *  sets or unset a bit in a bit-vector
 */
static int setbit(reg_mask_t *s, int n, int v)
{
    int p;

    assert(s);
    assert(ir_cur && n < ir_cur->x.nreg);
    assert((v & 1) == v);

    p = BIT(s, n);
    if (v)
        byte(s)[n/8] |= 1U << (n%8);
    else
        byte(s)[n/8] &= ~(1U << (n%8));

    return p;
}


/*
 *  sets bits in a bit-vector (va_list version)
 */
static int setbitsv(reg_mask_t *s, va_list ap)
{
    int n, p = 0;

    while ((n = va_arg(ap, int)) >= 0)
        p |= setbit(s, n, 1);

    return p;
}


/*
 *  creates a register mask
 */
reg_mask_t *(reg_mask)(arena_t *a, int n, ...)
{
    reg_mask_t *m;
    va_list ap;

    assert(a);

    va_start(ap, n);
    m = newbitv(a);
    if (n >= 0) {
        setbit(m, n, 1);
        setbitsv(m, ap);
    }
    va_end(ap);

    return m;
}


/*
 *  clears a register mask
 */
void (reg_mclear)(reg_mask_t *p)
{
    assert(p);
    assert(ir_cur);

    memset(p, 0, nbyte(ir_cur->x.nreg));
}


/*
 *  sets all bits in a register mask
 */
void (reg_mfill)(reg_mask_t *p)
{
    assert(p);
    assert(ir_cur);

    memset(p, 0xff, nbyte(ir_cur->x.nreg));
}


/*
 *  makes a backup of a register mask into strg_func
 */
reg_mask_t *(reg_mbackup)(const reg_mask_t *p)
{
    assert(p);
    assert(ir_cur);

    return memcpy(newbitv(strg_func), p, nbyte(ir_cur->x.nreg));
}


/*
 *  restores a register mask
 */
reg_mask_t *(reg_mrestore)(reg_mask_t *t, const reg_mask_t *s)
{
    assert(t);
    assert(s);
    assert(ir_cur);

    return memcpy(t, s, nbyte(ir_cur->x.nreg));
}


/*
 *  constructs a register symbol
 */
sym_t *(reg_new)(const char *f, int n, int set, ...)
{
    sym_t *s;
    va_list ap;

    assert(f);
    assert(n >= 0);
    assert(set < REG_SMAX);

    va_start(ap, set);
    s = ARENA_CALLOC(strg_perm, 1, sizeof(*s));
    s->x.name = gen_sfmt(strlen(f) + BUFN, f, n);
    s->x.regnode = ARENA_CALLOC(strg_perm, 1, sizeof(*s->x.regnode));
    s->x.regnode->set = set;
    s->x.regnode->num = n;
    s->x.regnode->bv = newbitv(strg_perm);
    setbitsv(s->x.regnode->bv, ap);
    va_end(ap);

    assert(!s->sclass);
    return s;
}


/*
 *  constructs a wildcard symbol
 */
sym_t *(reg_wildcard)(sym_t *pp[])
{
    sym_t *s;

    s = ARENA_CALLOC(strg_perm, 1, sizeof(*s));
    s->x.name = "wildcard";
    s->x.wildcard = pp;

    return s;
}


/*
 *  checks if a register is marked as used in a mask
 */
static int inuse(const reg_mask_t *r, const reg_mask_t *m)
{
    int i;
    unsigned long n = 0;

    assert(r);
    assert(m);
    assert(ir_cur);

    for (i = 0; i < nword(ir_cur->x.nreg); i++)
        n |= r[i] & ~m[i];

    return !!n;
}


/*
 *  resets bits in a mask for a register
 */
static void maskoff(reg_mask_t *m, const reg_mask_t *r)
{
    int i;

    assert(m);
    assert(r);
    assert(ir_cur);

    for (i = 0; i < nword(ir_cur->x.nreg); i++)
        m[i] &= ~r[i];
}


/*
 *  sets bits in a mask for a register
 */
static void maskon(reg_mask_t *m, const reg_mask_t *r)
{
    int i;

    assert(m);
    assert(r);
    assert(ir_cur);

    for (i = 0; i < nword(ir_cur->x.nreg); i++)
        m[i] |= r[i];
}


/*
 *  assigns a fixed register
 */
static sym_t *fixedreg(sym_t *r)
{
    int set;

    assert(r);
    assert(r->x.regnode);

    set = r->x.regnode->set;
    if (inuse(r->x.regnode->bv, reg_fmask[set]))
        return NULL;

    maskoff(reg_fmask[set], r->x.regnode->bv);
    maskon(reg_umask[set], r->x.regnode->bv);

    return r;
}


/*
 *  assigns a register from a register set
 */
static sym_t *askreg(sym_t *rs, reg_mask_t *ms[], reg_mask_t *em[])
{
    int i;

    assert(rs);
    assert(ms);
    assert(ir_cur);

    if (!rs->x.wildcard)
        return fixedreg(rs);

    for (i = ir_cur->x.nreg-1; i >= 0; i--) {
        sym_t *r = rs->x.wildcard[i];
        if (r && (assert(r->x.regnode), !inuse(r->x.regnode->bv, ms[r->x.regnode->set])) &&
            (!em || !inuse(r->x.regnode->bv, em[r->x.regnode->set])) &&
            fixedreg(r))
            return r;
    }

    return NULL;
}


/*
 *  allocates a register variable
 */
int (reg_askvar)(sym_t *s, sym_t *rs)
{
    sym_t *r;

    assert(s);

    if (s->sclass != LEX_REGISTER)
        return 0;
    if (!TY_ISSCALAR(s->type)) {
        s->sclass = LEX_AUTO;
        return 0;
    }
    if (s->u.t.cse) {
        s->x.name = "?";
        return 1;
    }
    if ((r=askreg(rs, reg_vmask, NULL)) != NULL) {
        s->x.regnode = r->x.regnode;
        s->x.regnode->vbl = s;
        s->x.name = r->x.name;
        DEBUG(printreg(stderr, "= allocating %s to %s\n", r->x.name, s->name));
        return 1;
    }
    s->sclass = LEX_AUTO;
    return 0;
}


/*
 *  compares two masks to check if they share the same register
 */
static int sharem(const reg_mask_t *m1, const reg_mask_t *m2)
{
    int i;
    unsigned n = 0;

    assert(m1);
    assert(m2);
    assert(ir_cur);

    for (i = 0; i < nword(ir_cur->x.nreg); i++)
        n |= m1[i] & m2[i];

    return !!n;
}


/*
 *  compares two symbols to check if they share the same register
 */
int (reg_shares)(const sym_t *s, const sym_t *t)
{
    assert(s);
    assert(s->x.regnode);
    assert(t);
    assert(t->x.regnode);

    if (s->x.regnode->set == t->x.regnode->set)
        return sharem(s->x.regnode->bv, t->x.regnode->bv);

    return 0;
}


/*
 *  sets the precluding mask
 */
void (reg_pmset)(dag_node_t *p, const sym_t *r)
{
    assert(p);
    assert(r);
    assert(r->x.regnode);
    assert(!p->sym[REG_RX] ||                                                 /* wildcard */
           (!p->sym[REG_RX]->x.regnode && p->sym[REG_RX] != r) ||             /* cse */
           (p->sym[REG_RX]->x.regnode && !reg_shares(p->sym[REG_RX], r)));    /* register */

    if (!p->x.pmask)
        p->x.pmask = reg_mbackup(r->x.regnode->bv);
    else
        maskon(p->x.pmask, r->x.regnode->bv);
}


/*
 *  sets the precluding mask for a register-targetted node with a '?' template
 */
void (reg_pmask)(dag_node_t *p)
{
    int i;
    const sym_t *s, *t;

    assert(p);
    s = p->sym[REG_RX];
    assert(s && s->x.regnode);
    assert(ir_cur);

    FORXKIDS(p, 1) {
        if (s->x.regnode->set != ir_cur->x.rmaps(p->op))
            continue;
        t = p->x.kid[i]->sym[REG_RX];
        assert(t && (t->x.wildcard || t->u.t.cse || (t->x.regnode && !reg_shares(s, t))));
        if (t->x.wildcard) {
            DEBUG(fprintf(stderr, "= pmask for (%p)->x.kid[%d] = (%p) to !%s\n",
                          (void *)p, i, (void *)p->x.kid[i], s->x.name));
            reg_pmset(p->x.kid[i], s);
        } else if (t->u.t.cse) {
            dag_node_t *f;
            FIRSTUSE(t, f);
            DEBUG(fprintf(stderr, "= pmask first use (%p) for (%p)->x.kid[%d] = (%p) to !%s\n",
                          (void *)f, (void *)p, i, (void *)p->x.kid[i], s->x.name));
            reg_pmset(f, s);
        } else
            assert(t->x.regnode && !reg_shares(s, t));
    }
}


/*
 *  checks if a dag node uses a register
 */
static int use(const dag_node_t *p, const sym_t *r)
{
    int i;
    const dag_node_t *q;

    assert(p);

    FORXKIDS(p, 0) {
        q = p->x.kid[i];
        if (q->x.f.registered && reg_shares(q->sym[REG_RX], r))
            return 1;
    }

    return 0;
}


/*
 *  selects the best register to spill
 */
static sym_t *spillee(sym_t *set, reg_mask_t *em[], dag_node_t *p)
{
    int i, dist = -1;
    sym_t *best = NULL;

    assert(set);
    assert(p);
    assert(ir_cur);

    if (!set->x.wildcard)
        return set;

    for (i = 0; i < ir_cur->x.nreg; i++) {
        sym_t *r = set->x.wildcard[i];
        if (r && !REG_ISRVAR(r) &&
            (assert(r->x.regnode), sharem(r->x.regnode->bv, reg_tmask[r->x.regnode->set])) &&
            (!em || sharem(r->x.regnode->bv, em[r->x.regnode->set]))) {
            int d = 0;
            dag_node_t *q = p;
            for (; q && !use(q, r); q = q->x.next)
                d++;
            if (q && d > dist) {
                dist = d;
                best = r;
            }
        }
    }

    return best;
}


/*
 *  extracts a suffix from an op code for ASGN and INDIR
 */
static int getty(int op)
{
    op = op_tyscode(op);
    if (OP_ISUINT(op))
        op = OP_I + op_scode(op);

    return op;
}


/*
 *  generates spill nodes
 */
static void genspill(sym_t *r, dag_node_t *f, sym_t *t)
{
    int ty;
    sym_t *s;
    dag_node_t *p, *q;

    assert(r);
    assert(r->x.regnode);
    assert(f);
    assert(t);

    DEBUG(fputs(">> genspill starts\n", stderr));
    DEBUG(fprintf(stderr, "= spilling %s to %s\n", r->x.name, t->x.name));
    DEBUG(fputs("= genspill for:\n", stderr));
    DEBUG(dag_print(f, stderr, 1));

    s = ARENA_CALLOC(strg_func, 1, sizeof(*s));
    s->sclass = LEX_REGISTER;
    s->name = s->x.name = r->x.name;    /* sets s->name for debugging */
    s->x.regnode = r->x.regnode;
    r->x.regnode->vbl = s;

    ty = getty(f->op);
    p = dag_newnode(OP_ASGN+ty,
                    dag_newnode(op_addr(L), NULL, NULL, t),
                    dag_newnode(OP_INDIR+ty,
                                dag_newnode(op_addr(L), NULL, NULL, s),
                                NULL,
                                NULL),
                    NULL);
    p->x.f.listed = 1;    /* to avoid register allocation */
    p->x.f.spill = 1;     /* equate() starts from p not from f */
    DEBUG(fputs("= node to insert:\n", stderr));
    DEBUG(dag_print(p, stderr, 1));

    gen_rewrite(p);
    gen_prune(p, &q, &q+1);
    q = f->x.next;
    gen_linearize(p, q);
    gen_wildcard(p);
    for (p = f->x.next; p != q; p = p->x.next)
        reg_alloc(p);
    r->x.regnode->vbl = NULL;

    DEBUG(fputs("<< genspill ends\n", stderr));
}


/*
 *  adjusts kid for x.kid to be replaced
 */
static void reprune(dag_node_t *p, const dag_node_t *k, dag_node_t *q)
{
    int i;
    dag_node_t **ppv;

    assert(k);
    assert(q);
    assert(ir_cur);

    if (!p)
        return;

    if (p->x.vr)
        for (ppv = p->x.vr; *ppv; ppv++)
            if (*ppv == k)
                *ppv = q;
    for (i = 0; i < NELEM(p->kid); i++)
        if (p->kid[i] == k) {
            DEBUG(fprintf(stderr, "= reprune changes (%p)->kid[%d] from %p to %p\n",
                          (void *)p, i, (void *)k, (void *)q));
            p->kid[i] = q;
            ir_cur->x.target(p);
        } else
            reprune(p->kid[i], k, q);
}


/*
 *  generates reload nodes
 */
static void genreload(dag_node_t *p, sym_t *t, int i)
{
    int ty;
    dag_node_t *q, *dummy;

    assert(p);
    assert(p->x.kid[i]);
    assert(t);

    DEBUG(fputs(">> genreload starts\n", stderr));
    DEBUG(fprintf(stderr, "= replacing %p with reload from %s\n", (void *)p->x.kid[i], t->x.name));
    DEBUG(fputs("= genreload for:\n", stderr));
    DEBUG(dag_print(p->x.kid[i], stderr, 1));

    ty = getty(p->x.kid[i]->op);
    q = dag_newnode(OP_INDIR+ty,
                    dag_newnode(op_addr(L), NULL, NULL, t),
                    NULL, NULL);
    q->count = 1;
    DEBUG(fputs("= node to substitute:\n", stderr));
    DEBUG(dag_print(q, stderr, 1));

    gen_rewrite(q);
    gen_prune(q, &dummy, &dummy+1);
    reprune(p, p->x.kid[i], q);
    gen_prune(p, &dummy, &dummy+1);
    gen_linearize(p->x.kid[i], p);
    gen_wildcard(p->x.kid[i]);

    DEBUG(fputs("<< genreload ends\n", stderr));
}


/*
 *  frees a register
 */
static void putreg(const sym_t *r)
{
    assert(r);
    assert(r->x.regnode);

    maskon(reg_fmask[r->x.regnode->set], r->x.regnode->bv);
    DEBUG(printreg(stderr, " - freed %s\n", r->x.name));
}


/*
 *  spills a register;
 *  spilling renders x.lastuse useless; see equate() in gen.c
 */
static void spillr(sym_t *r, dag_node_t *p)
{
    int i;
    sym_t *t;
    dag_node_t *f;

    assert(r);
    assert(p);

    FIRSTUSE(r, f);
    do {
        (f) = (r)->x.lastuse, assert(f);
        while ((f)->x.prevuse)
            assert((f)->sym[REG_RX] == (r)),
            (f) = (f)->x.prevuse;
    } while(0);


    assert(f->x.f.registered && !READ(f));
#ifndef NDEBUG
    {    /* genspill node must precede spilling node */
        dag_node_t *d;
        for (d = f; d && d != p; d = d->x.next)
            continue;
        assert(d);
    }
#endif    /* !NDEBUG */
    t = sym_new(SYM_KTEMPB, LEX_AUTO, f->op);
    genspill(r, f, t);
    while ((p = p->x.next) != NULL)
        FORXKIDS(p, 0) {
            dag_node_t *k = p->x.kid[i];
            if (k->x.f.registered && k->sym[REG_RX] == r)
                genreload(p, t, i);
        }
    putreg(r);
}


/*
 *  spills registers
 */
void (reg_spill)(const reg_mask_t *m, int set, dag_node_t *p)
{
    int i;
    dag_node_t *q;

    assert(set < REG_SMAX);

    DEBUG(fputs(">> spill starts\n", stderr));
    maskon(reg_umask[set], m);
    if (inuse(m, reg_fmask[set]))
        for (q = p; q; q = q->x.next)
            FORXKIDS(q, 0) {
                sym_t *r = q->x.kid[i]->sym[REG_RX];
                assert(r);
                if (q->x.kid[i]->x.f.registered &&
                    (assert(r->x.regnode), r->x.regnode->set == set) &&
                    sharem(r->x.regnode->bv, m))
                    spillr(r, p);
            }
    DEBUG(fputs("<< spill ends\n", stderr));
}


/*
 *  requests a register
 */
static sym_t *getreg(sym_t *rs, reg_mask_t *ms[], reg_mask_t *em[], dag_node_t *p)
{
    sym_t *r = askreg(rs, ms, em);

    if (!r) {
        r = spillee(rs, em, p);
        assert(r && r->x.regnode && !r->x.regnode->vbl);
        reg_spill(r->x.regnode->bv, r->x.regnode->set, p);
        r = askreg(rs, ms, em);
        assert(r);
    }
    assert(r->x.regnode);
    assert(!r->x.regnode->vbl);

    return r;
}


/*
 *  allocates a register for a dag node
 */
void (reg_alloc)(dag_node_t *p)
{
    static reg_mask_t *emask[REG_SMAX];

    int i;
    sym_t *set;
    reg_mask_t **em = NULL;

    assert(p);
    DEBUG(fprintf(stderr, "= rallocing %p\n", (void *)p));
    FORXKIDS(p, 0) {
        sym_t *r = p->x.kid[i]->sym[REG_RX];
        assert(p->x.kid[i]->x.f.registered && r);
        if (!REG_ISRVAR(r) && r->x.lastuse == p->x.kid[i])
            putreg(r);
    }

    assert(ir_cur);
    if (!p->x.f.registered && REG_NEED(p) && (set = ir_cur->x.rmapw(p->op)) != NULL) {
        sym_t *sym = p->sym[REG_RX];
        assert(sym);
        if (!sym->u.t.cse)
            set = sym;
        if (set->x.wildcard || !REG_ISRVAR(set)) {    /* wildcards have no regnode set */
            sym_t *r;
            int cond = (NELEM(p->x.kid) > 1 &&
                        ir_cur->x.rule[p->x.rn[p->x.inst]].tmpl[0] == '?' && p->x.kid[1]);
            if (p->x.pmask)
                cond |= 0x02;
            if (cond) {
                if (!emask[0])
                    for (i = 0; i < REG_SMAX; i++)
                        emask[i] = newbitv(strg_perm);
                for (i = 0; i < REG_SMAX; i++)
                    reg_mfill(emask[i]);
                if (cond & 0x01) {
                    FORXKIDS(p, 1) {
                        r = p->x.kid[i]->sym[REG_RX];
                        assert(p->x.kid[i]->x.f.registered);
                        assert(r && r->x.regnode);
                        assert(sym->x.wildcard ||                           /* wildcard */
                               (!sym->x.regnode && sym != r) ||             /* cse */
                               (sym->x.regnode && !reg_shares(sym, r)));    /* register */
                        maskoff(emask[r->x.regnode->set], r->x.regnode->bv);
                    }
                    DEBUG(fprintf(stderr, " - extramask:"));
                    DEBUG(printbv(emask, stderr));
                }
                if (cond & 0x02) {
                    maskoff(emask[ir_cur->x.rmaps(p->op)], p->x.pmask);
                    DEBUG(fprintf(stderr, " - pmask:"));
                    DEBUG(printbv(emask, stderr));
                }
                em = emask;
            }
            r = getreg(set, reg_tmask, em, p);
            if (sym->u.t.cse) {
                dag_node_t *q;
                r->x.lastuse = sym->x.lastuse;
                for (q = sym->x.lastuse; q; q = q->x.prevuse) {
                    reg_set(q, r);
                    q->x.f.registered = 1;
                    if (q->x.f.copy)
                        q->x.f.equatable = 1;
                }
            } else {
                reg_set(p, r);
                r->x.lastuse = p;
            }
            DEBUG(printreg(stderr, " - allocated %s to %p\n", r->x.name, (void *)p));
        } else {    /* register variable or pseudo one from spill */
            assert(sym->x.regnode && (sym->x.regnode->vbl == sym ||
                                      sym->x.regnode->vbl->name == sym->x.name));
            DEBUG(fprintf(stderr, " - %s targeted to %p\n", sym->x.name, (void *)p));
#ifndef NDEBUG
            if (ir_cur->x.rule[p->x.rn[p->x.inst]].tmpl[0] == '?' &&
                NELEM(p->x.kid) > 1 && p->x.kid[1])
                FORXKIDS(p, 1) {
                    assert(!reg_shares(sym, p->x.kid[i]->sym[REG_RX]));
                }
#endif    /* !NDEBUG */
        }
    }
    p->x.f.registered = 1;
    ir_cur->x.clobber(p);

    if (p->x.f.listed && REG_ROOT(p) && p->sym[REG_RX])
        putreg(p->sym[REG_RX]);
}


/*
 *  assigns a register to a dag node;
 *  must not be used by ir_cur->x.target()
 */
void (reg_set)(dag_node_t *p, sym_t *r)
{
    assert(p);

    p->sym[REG_RX] = r;
}


/*
 *  assigns a register if not targeted;
 *  ir_cur->x.target() has to use this
 */
void (reg_setnt)(dag_node_t *p, sym_t *r)
{
    assert(p);
    assert(r);

    if (!p->sym[REG_RX] || p->sym[REG_RX]->x.name[0] == '?')    /* wildcard or cse temp */
        reg_set(p, r);
}


/*
 *  adds a LOAD node while register (p)targetting
 */
static void addload(dag_node_t *p, int n, sym_t *r)
{
    dag_node_t *q;

    assert(p);
    assert(p->kid[n]);
    assert(r);

    q = p->kid[n];
    assert(q->count == 1);
    q = dag_newnode(OP_LOAD+op_tyscode(q->op), q, NULL, q->sym[0]);
    q->count = 1;
    if (r->u.t.cse == p->kid[n])
        r->u.t.cse = q;
    p->kid[n] = q;
}


/*
 *  performs register targetting
 */
void (reg_target)(dag_node_t *p, int n, sym_t *r)
{
    assert(p);
    assert(p->kid[n]);
    assert(r);
    assert(r->sclass == LEX_REGISTER || !r->x.wildcard);

    if (p->kid[n]->sym[REG_RX] && r != p->kid[n]->sym[REG_RX])
        addload(p, n, r);
    reg_set(p->kid[n], r);
    DEBUG(fprintf(stderr, "= target (%p)->x.kid[%d] = (%p) to %s\n",
                  (void *)p, n, (void *)p->kid[n], r->x.name));
}


/*
 *  performs register precluding
 */
void (reg_ptarget)(dag_node_t *p, int n, sym_t *r)
{
    sym_t *s;

    assert(p);
    assert(p->kid[n]);
    assert(r);

    s = p->kid[n]->sym[REG_RX];
    if (s && (r == s ||                               /* cse */
              (s->x.regnode && reg_shares(s, r))))    /* register */
        addload(p, n, r);
    reg_pmset(p->kid[n], r);
    DEBUG(fprintf(stderr, "= ptarget (%p)->kid[%d] = (%p) to !%s\n",
                  (void *)p, n, (void *)p->kid[n], r->x.name));
}


#ifndef NDEBUG
/*
 *  prints register masks for debugging
 */
static void printbv(reg_mask_t *ms[], FILE *fp)
{
    int i, j;

    assert(ms);
    assert(fp);
    assert(ir_cur);

    for (i = 0; i < REG_SMAX; i++) {
        putc(' ', fp);
        for (j = 0; j < ir_cur->x.nreg; j++)
            fprintf(fp, "%d", BIT(ms[i], j));
    }
    putc('\n', fp);
}


/*
 *  prints free/used register masks
 */
static void printreg(FILE *fp, const char *fmt, ...)
{
    va_list ap;

    assert(fp);
    assert(fmt);

    va_start(ap, fmt);
    vfprintf(fp, fmt, ap);
    va_end(ap);

    fprintf(fp, "  - freemask:");
    printbv(reg_fmask, fp);
    fprintf(fp, "  - usedmask:");
    printbv(reg_umask, fp);
}
#endif    /* !NDEBUG */

/* end of reg.c */
