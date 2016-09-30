/*
 *  macro for preprocessing
 */

#include <stddef.h>        /* size_t, NULL */
#include <string.h>        /* strcmp, strlen, memcpy */
#include <cbl/arena.h>     /* arena_t, ARENA_ALLOC, ARENA_CALLOC */
#include <cbl/assert.h>    /* assert */
#include <cbl/memory.h>    /* MEM_NEW, MEM_CALLOC, MEM_FREE */
#include <cdsl/hash.h>     /* hash_string, hash_new */
#ifndef NDEBUG
#include <stdio.h>         /* FILE, fprintf, fputs */
#endif    /* !NDEBUG */

#include "common.h"
#include "err.h"
#include "lex.h"
#include "lst.h"
#include "lmap.h"
#include "strg.h"
#include "util.h"
#include "mcr.h"

#define MTAB  128      /* initial size of macro table; must be power of 2 */
#define MAXMT 32768    /* max size of macro table */

#define MAXDS 6        /* max number of successive #'s to fully diagnose */

#define EXPANDING(p) ((p) && (p)->count > 0)                 /* checks if macro being expanded */
#define SPELL(t, s)  ((t)->spell = (s), (t)->f.alloc = 0)    /* sets spelling of token */
#define isempty(t)   (*(t)->spell == '\0')                   /* true if empty token */

/* (predefined macros) checks if predefined macro */
#define ISPREDMCR(n) ((n)[0] == '_' && (n)[1] == '_')


/* (command list) macros from command line */
struct cmdlist {
    int del;            /* 0: to add, 1: to remove */
    const char *arg;    /* argument string */
};

/* parameter list */
struct plist {
    const char *chn;       /* parameter name (clean, hashed) */
    lex_t **rlist;         /* replacement list */
    lex_t *elist;          /* expanded replacement list */
    struct plist *next;    /* next entry */
};

/* parameter expansion list */
struct pelist {
    const char *chn;        /* parameter name (clean, hashed) */
    int expand;             /* # of occurrences for expansion */
    struct pelist *next;    /* next entry */
};


/* macro table */
static struct {
    size_t u, n;    /* # of used/total buckets */
    struct mtab {
        const char *chn;      /* macro name (clean, hashed) */
        const lmap_t *pos;    /* definition locus */
        lex_t **rlist;        /* replacement list */
        struct {
            unsigned flike:  1;    /* function-like */
            unsigned sharp:  1;    /* has # or ## */
            unsigned predef: 1;    /* predefined macro */
        } f;
        struct {
            int argno;            /* # of arguments */
            lex_t **param;        /* parameters */
            struct pelist *pe;    /* parameter expansion list */
        } func;                   /* function-like macro */
        struct mtab *link;    /* hash chain */
    } **t;
} mtab;

/* expanding macro list */
static struct emlist {
    const char *chn;        /* macro name (clean, hashed) */
    int count;              /* nesting count */
    struct emlist *next;    /* next entry */
} *em;

static int diagds;       /* true if issueing ERR_PP_ORDERDS is enabled */
static int nppname;      /* number of macros defined */
static int mlev;         /* nesting levels of macro expansions */


/*
 *  (parameter expansion list) adds a parameter
 */
static struct pelist *peadd(struct pelist *l, const lex_t *t, int *found)
{
    const char *chn;
    struct pelist *p;

    assert(t);
    assert(found);

    chn = hash_string(LEX_SPELL(t));
    for (p = l; p; p = p->next)
        if (p->chn == chn) {
            *found = 1;
            return l;
        }

    p = ARENA_ALLOC(strg_perm, sizeof(*p));
    p->chn = chn;
    p->expand = 0;
    p->next = l;
    *found = 0;

    return p;
}


/*
 *  (parameter expansion list) looks up a parameter
 */
static struct pelist *pelookup(struct pelist *p, const lex_t *t)
{
    const char *chn;

    assert(t);

    chn = hash_string(LEX_SPELL(t));
    for (; p; p = p->next)
        if (p->chn == chn)
            break;

    return p;
}


/*
 *  (expanding macro list) adds an identifier
 */
void (mcr_eadd)(const char *chn)
{
    struct emlist *p;

    assert(chn);

    for (p = em; p; p = p->next)
        if (p->chn == chn)
            break;

    if (!p) {
        MEM_NEW(p);
        p->chn = chn;
        p->count = 0;
        p->next = em;
    }
    p->count++;
    em = p;
}


/*
 *  (expanding macro list) looks up an identifier
 */
static struct emlist *elookup(const lex_t *t)
{
    const char *chn;
    struct emlist *p;

    assert(t);

    chn = hash_string(LEX_SPELL(t));
    for (p = em; p; p = p->next)
        if (p->chn == chn)
            break;

    return p;
}


/*
 *  (expanding macro list) removes an identifier
 */
void (mcr_edel)(const char *chn)
{
    struct emlist **p, *q;

    assert(chn);

    for (p = &em; *p; p = &(*p)->next)
        if ((*p)->chn == chn && --(*p)->count == 0) {
            q = *p;
            *p = q->next;
            MEM_FREE(q);
            break;
        }
}


/*
 *  (parameter list) adds a macro parameter
 */
static struct plist *padd(struct plist *pl, const lex_t *t, lex_t **rl, lex_t *el)
{
    const char *chn;
    struct plist *p;

    assert(t);

    chn = hash_string(LEX_SPELL(t));
    p = ARENA_ALLOC(strg_perm, sizeof(*p));
    p->chn = chn;
    p->rlist = rl;
    p->elist = el;
    p->next = pl;

    return p;
}


/*
 *  (parameter list) look up a parameter
 */
static struct plist *plookup(struct plist *p, const lex_t *t)
{
    const char *chn;

    assert(t);

    chn = hash_string(LEX_SPELL(t));
    for (; p; p = p->next)
        if (p->chn == chn)
            break;

    return p;
}


/*
 *  (macro table) adjusts the size of a hash bucket
 */
static void resize(void)
{
    unsigned h;
    struct mtab *p, *q, **nt;
    size_t i, nn = mtab.n * 2;

    nt = MEM_CALLOC(nn, sizeof(*nt));

    for (i = 0; i < mtab.n; i++)
        for (p = mtab.t[i]; p; p = q) {
            q = p->link;
            h = hashkey(p->chn, nn);
            p->link = nt[h];
            nt[h] = p;
        }

    MEM_FREE(mtab.t);
    mtab.t = nt;
    mtab.n = nn;
}


/*
 *  compares two token lists for equality
 */
static int eqtlist(lex_t *p[], lex_t *q[])
{
    assert(p);
    assert(q);

    while (*p && *q && (*p)->id == (*q)->id && strcmp(LEX_SPELL(*p), LEX_SPELL(*q)) == 0)
        p++, q++;

    return (!*p && !*q);
}


/*
 *  (macro table) adds an identifier
 */
static struct mtab *add(const char *cn, const lmap_t *pos, lex_t *l[], lex_t *param[])
{
    unsigned h;
    const char *chn;
    struct mtab *p;

    assert(cn);
    assert(pos);

    chn = hash_string(cn);
    h = hashkey(chn, mtab.n);
    for (p = mtab.t[h]; p; p = p->link)
        if (p->chn == chn) {
            if ((p->f.flike ^ !!param) || !eqtlist(p->rlist, l) ||
                (param && !eqtlist(p->func.param, param))) {
                err_dpos(pos, ERR_PP_MCRREDEF, chn);
                err_dpos(p->pos, ERR_PP_PREVDEF);
            }
            return NULL;
        }
    if (++mtab.u*3 > mtab.n*2 && mtab.n < MAXMT) {
        resize();
        h = hashkey(chn, mtab.n);
    }

    p = ARENA_CALLOC(strg_perm, 1, sizeof(*p));
    p->chn = chn;
    p->pos = pos;
    p->rlist = l;
    p->f.flike = !!param;
    p->func.argno = -1;
    p->func.param = param;
    p->link = mtab.t[h];
    mtab.t[h] = p;

    return p;
}


/*
 *  (macro table) looks up an identifier
 */
static struct mtab *lookup(const char *cn)
{
    unsigned h;
    const char *chn;
    struct mtab *p;

    assert(cn);

    chn = hash_string(cn);
    h = hashkey(chn, mtab.n);
    for (p = mtab.t[h]; p; p = p->link)
        if (p->chn == chn)
            break;

    return p;
}


/*
 *  (macro table) checks if an identifier has been #defined
 */
int (mcr_redef)(const char *cn)
{
    return !!lookup(cn);
}


/*
 *  (macro table) #undefines a macro
 */
void (mcr_del)(const char *cn, const lmap_t *pos)
{
    struct mtab *p = lookup(cn);

    assert(pos);

    if (p) {
        if (p->f.predef)
            err_dpos(pos, ERR_PP_PMCRUNDEF, cn);
        else {
            p->chn = NULL;
            nppname--;
            assert(nppname >= 0);
        }
    } else
        err_dpos(pos, ERR_PP_UNDEFMCR, cn);
}


/*
 *  checks if parameters need to be expanded
 */
static void chkexp(struct pelist *l, lex_t *p[])
{
    int first = 0;
    struct pelist *q;
    lex_t *t, *pt = NULL;

    assert(p);

    while (*p) {
        t = *p;
        switch(t->id) {
            case LEX_ID:
                if (pt) {
                    if (pt->id == LEX_STROP) {
                        pelookup(l, t)->expand--;
                        t = NULL;
                        break;
                    } else if (pt->id == LEX_PASTEOP) {
                        if ((q = pelookup(l, t)) != NULL)
                            q->expand--;
                    } else
                        first = 0;
                }
                break;
            case LEX_PASTEOP:
                if (!first) {
                    first = 1;
                    if (pt && pt->id == LEX_ID && (q = pelookup(l, pt)) != NULL)
                        q->expand--;
                }
                break;
            default:
                first = 0;
                break;
        }
        pt = t;
        while (*++p && (*p)->id == LEX_SPACE)
            continue;
    }
}


/*
 *  detects macro name conflicts
 */
static struct mtab *conflict(const char *chn)
{
    static struct ctab {
        const char *cname;
        const char *chn;
        struct ctab *link;
    } *ctab[16];

    unsigned h;
    const char *cname;
    struct ctab *p, *q;
    struct mtab *r;

    assert(chn);

    if (!main_opt()->std || snlen(chn, TL_INAME_STD) < TL_INAME_STD)
        return NULL;

    cname = hash_new(chn, TL_INAME_STD);
    h = hashkey(cname, NELEM(ctab));
    q = NULL, r = NULL;
    for (p = ctab[h]; p; p = p->link)
        if (cname == p->cname) {
            if (chn == p->chn)
                q = p;
            else if (!r)
                r = lookup(p->chn);
            if (r && q)
                return r;
        }

    if (!q) {
        p = ARENA_ALLOC(strg_perm, sizeof(*p));
        p->cname = cname;
        p->chn = chn;
        p->link = ctab[h];
        ctab[h] = p;
    }

    return r;
}


/*
 *  accepts and handles macro definitions;
 *  does what should be done in proc.c for command-line definitions
 */
lex_t *(mcr_define)(int cmd)
{
    int n = -1;
    int sharp = 0;
    lex_t *t, *l = NULL;
    const char *cn;
    const lmap_t *idpos;
    arena_t *strg;
    lex_t **param = NULL;
    struct pelist *pe = NULL;

    NEXTSP(t);    /* consumes define */
    if (t->id != LEX_ID) {
        err_dpos(t->pos, ERR_PP_NOMCRID);
        return t;
    }
    cn = LEX_SPELL(t);
    idpos = t->pos;
    strg = (mcr_redef(cn))? strg_line: strg_perm;
    t = lst_nexti();

    if (t->id == '(') {    /* function-like */
        int dup;
        lex_t *pl = NULL;
        const lmap_t *pos = t->pos;

        n = 0;
        NEXTSP(t);    /* consumes ( */
        while (t->id == LEX_ID) {
            pos = t->pos;
            if (n++ == TL_PARAMP_STD) {
                err_dpos(t->pos, ERR_PP_MANYPARAM);
                err_dpos(t->pos, ERR_PP_MANYPSTD, (long)TL_PARAMP_STD);
            }
            pe = peadd(pe, t, &dup);
            if (dup) {
                err_dpos(t->pos, ERR_PP_DUPNAME, pe->chn);
                return t;
            }
            pl = lst_append(pl, lst_copy(t, 1, strg));
            NEXTSP(t);    /* consumes id */
            if (t->id != ',')
                break;
            NEXTSP(t);    /* consumes , */
            if (t->id != LEX_ID) {
                err_dpos(t->pos, ERR_PP_NOPNAME);
                return t;
            }
        }
        if (t->id != ')') {
            err_dafter(pos, ERR_PP_NOPRPAREN);
            return t;
        }
        param = lst_toarray(pl, strg);
        t = lst_nexti();
    }

    /* replacement list */
    if (t->id != LEX_SPACE && t->id != LEX_NEWLINE && n < 0 && !cmd)
        err_dafter(idpos, ERR_PP_NOSPACE, cn);
    SKIPSP(t);
    while (t->id != LEX_NEWLINE && t->id != LEX_EOI) {
        if (t->id == LEX_SPACE) {
            lex_t *u;
            NEXTSP(u);    /* consumes space */
            if (u->id != LEX_NEWLINE && u->id != LEX_EOI) {
                SPELL(t, " ");
                l = lst_append(l, lst_copy(t, 1, strg));
            }
            t = u;
            continue;
        } else {
            l = lst_append(l, lst_copy(t, 1, strg));
            if (n > 0 && t->id == LEX_ID) {
                struct pelist *p = pelookup(pe, t);
                if (p)
                    p->expand++;
            } else if (t->id == LEX_SHARP || t->id == LEX_DSHARP) {
                lex_t *ts = l;    /* not t */
                t = lst_nexti();
                if (t->id == LEX_SPACE) {
                    lex_t *u;
                    NEXTSP(u);    /* consumes space */
                    if (u->id != LEX_NEWLINE && u->id != LEX_EOI) {
                        SPELL(t, " ");
                        l = lst_append(l, lst_copy(t, 1, strg));
                    }
                    t = u;
                }
                if (ts->id == LEX_DSHARP) {
                    if (l->next->id == LEX_DSHARP || (t->id == LEX_NEWLINE || t->id == LEX_EOI)) {
                        err_dpos(ts->pos, ERR_PP_DSHARPPOS);
                        return t;
                    } else if (t->id == LEX_DSHARP) {
                        err_dpos(t->pos, ERR_PP_TWODSHARP);
                        return t;
                    }
                    ts->id = LEX_PASTEOP;
                    sharp = 1;
                } else if (n >= 0) {
                    if (t->id != LEX_ID || !pelookup(pe, t)) {
                        err_dpos(ts->pos, ERR_PP_NEEDPARAM);
                        return t;
                    }
                    ts->id = LEX_STROP;
                    sharp = 1;
                }
                continue;
            }
        }
        t = lst_nexti();
    }

    {    /* installation */
        struct mtab *p;

        if (ISPREDMCR(cn)) {
            p = lookup(cn);
            if (p && p->f.predef) {
                err_dpos(idpos, ERR_PP_PMCRREDEF, cn);
                return t;
            }
        } else if (strcmp(cn, "defined") == 0) {
            err_dpos(idpos, ERR_PP_MCRDEF);
            return t;
        }
        p = add(cn, idpos, lst_toarray(l, strg), param);
        if (p) {
            if (nppname++ == TL_PPNAME_STD) {
                err_dpos(idpos, ERR_PP_MANYPPID);
                err_dpos(idpos, ERR_PP_MANYPPIDSTD, (long)TL_PPNAME_STD);
            }
            if (n >= 0) {
                p->func.argno = n;
                p->func.pe = pe;
                if (sharp && n > 0)
                    chkexp(pe, p->rlist);
            }
            if (sharp)
                p->f.sharp = 1;
            if ((p = conflict(p->chn)) != NULL) {
                err_dpos(idpos, ERR_PP_LONGID);
                err_dpos(idpos, ERR_PP_LONGIDSTD, (long)TL_INAME_STD);
                err_dpos(p->pos, ERR_PP_SEEID, p->chn);
            }
        }
    }

    return t;
}


#define ISNL(nl) (t->id == LEX_NEWLINE && ((nl)=t, !lex_direc))

/*
 *  skips spaces and newlines in macro arguments
 */
static lex_t *nextspnl(lex_t **pnl)
{
    lex_t *t;

    assert(pnl);

    while ((t = lst_nexti())->id == LEX_SPACE)
        continue;
    if (ISNL(*pnl)) {
        while ((t = lst_nexti())->id == LEX_SPACE || ISNL(*pnl))
            continue;
        if (t->id == LEX_SHARP)
            err_dpos(t->pos, ERR_PP_DIRECINARG);
    }

    return t;
}


/*
 *  expands macros from arguments
 */
static lex_t *exparg(lex_t *l, const lmap_t *pos)
{
    lex_t *t;

    if (!l)
        return NULL;

    lst_push(l);
    if (mlev++ == 0)
        lmap_head = pos;    /* set */

    while ((t = lst_nexti())->id != LEX_EOI) {
        assert(t->id != LEX_NEWLINE);
        if (t->id == LEX_ID)
            mcr_expand(t);
    }
    lst_flush(mlev, 1);

    if (--mlev == 0) {
        lmap_head = lmap_head->from;    /* restore */
        assert(lmap_head);
    }
    return lst_pop();
}


/*
 *  recognizes arguments for function-like macros
 */
static struct plist *recarg(struct mtab *p, const lmap_t **ppos)
{
    int errarg = 0;
    int level = 1, n = 0;
    lex_t *t, *nl = NULL;
    lex_t *tl = NULL, **rl;
    struct plist *pl = NULL;
    const lmap_t *pos, *prnpos;

    assert(p);
    assert(ppos);

    pos = (mlev == 0)? *ppos: lmap_head;
    while ((t = lst_nexti())->id != '(')
        continue;
    prnpos = t->pos;
    t = nextspnl(&nl);    /* consumes ( */

    while (t->id != LEX_EOI && (t->id != LEX_NEWLINE || (nl=t, !lex_direc))) {
        if (t->id == LEX_SPACE || ISNL(nl)) {
            lex_t *u = nextspnl(&nl);    /* consumes space or newline */
            if (!(level == 1 && (u->id == ',' || u->id == ')')) && u->id != LEX_EOI &&
                (!lex_direc || u->id != LEX_NEWLINE)) {
                t->id = LEX_SPACE;
                SPELL(t, " ");
                tl = lst_append(tl, lst_copy(t, 1, strg_line));
            }
            t = u;
            continue;
        }
        switch(t->id) {
            case ')':
                level--;
                /* no break */
            case ',':
                if (level > (t->id == ',')) {
                    assert(tl);
                    tl = lst_append(tl, lst_copy(t, 1, strg_line));
                } else {
                    if (t->id == ',' || p->func.argno > 0) {
                        if (!tl) {
                            if (n++ == p->func.argno) {
                                err_dpos(lmap_copy(t->pos, pos, strg_line), ERR_PP_MANYARG1,
                                         p->chn);
                                errarg = 1;
                            } else if (n <= p->func.argno)
                                err_dpos(t->pos, ERR_PP_EMPTYARG, p->chn);
                            if (n == TL_ARGP_STD+1 && !errarg) {
                                const lmap_t *tpos = lmap_copy(t->pos, pos, strg_line);
                                err_dpos(tpos, ERR_PP_MANYARG2, p->chn);
                                err_dpos(tpos, ERR_PP_MANYARGSTD, (long)TL_ARGP_STD);
                            }
                        }
                        if (n <= p->func.argno) {
                            struct pelist *pe;
                            assert(n > 0);
                            pe = pelookup(p->func.pe, p->func.param[n-1]);
                            assert(pe);
                            rl = lst_toarray(tl, strg_line);    /* before exparg() */
                            pl = padd(pl, p->func.param[n-1], rl, (pe->expand)?
                                                                      exparg(tl, pos): NULL);
                        }
                        tl = NULL;
                    }
                    if (t->id == ')')
                        goto ret;
                    t = nextspnl(&nl);    /* consumes , */
                    continue;
                }
                break;
            case '(':
                level++;
            default:
                if (!tl) {
                    if (n++ == p->func.argno) {
                        err_dpos(lmap_copy(t->pos, pos, strg_line), ERR_PP_MANYARG1, p->chn);
                        errarg = 1;
                    }
                    if (n == TL_ARGP_STD+1 && !errarg) {
                        const lmap_t *tpos = lmap_copy(t->pos, pos, strg_line);
                        err_dpos(tpos, ERR_PP_MANYARG2, p->chn);
                        err_dpos(tpos, ERR_PP_MANYARGSTD, (long)TL_ARGP_STD);
                    }
                }
                tl = lst_append(tl, lst_copy(t, 1, strg_line));
                break;
        }
        t = lst_nexti();
    }

    ret:
        *ppos = lmap_range(*ppos, t->pos);
        if (level > 0)
            err_dpos(lmap_copy(prnpos, pos, strg_line), ERR_PP_UNTERMARG, p->chn);
        else if (n < p->func.argno) {
            err_dpos(lmap_copy(t->pos, pos, strg_line), ERR_PP_INSUFFARG, p->chn);
            while (n++ < p->func.argno)
                pl = padd(pl, p->func.param[n-1], lst_toarray(tl, strg_line), NULL);
        }

        if ((t->id == LEX_EOI || t->id == LEX_NEWLINE) && nl)
            lst_insert(lst_copy(nl, 1, strg_line));
        lst_discard(mlev, 1);    /* removes from macro name to end of invocation */

        return pl;
}

#undef ISNL


/*
 *  paints tokens being expanded in an expanded replacement list
 */
static void paint(lex_t *l)
{
    lex_t *t;
    struct emlist *pe;

    if (!l)
        return;

    t = l = l->next;
    do {
        if (t->id == LEX_ID) {
            pe = elookup(t);
            if (EXPANDING(pe))
                t->f.blue = 1;
        } else if (t->id == LEX_MCR)
            t->spell = NULL;    /* since painting starts after expansion of arguments,
                                   necessary to stop making expanding context */
        t = t->next;
    } while(t != l);
}


/*
 *  expands an identifier if it denotes a macro
 */
int (mcr_expand)(lex_t *t)
{
    struct mtab *p;
    struct emlist *pe;
    struct plist *pl = NULL, *r;
    lex_t *l = NULL, **q;
    const lmap_t *idpos;

    assert(t);

    p = lookup(LEX_SPELL(t));
    if (!p || t->f.blue)
        return 0;

    idpos = t->pos;
    /* lmap_head not set here to adjust idpos in recarg() */
    if (p->f.flike) {
        lex_t *u = lst_peeki();
        if (u->id == '(') {
            lst_flush(mlev, 0);    /* before macro name */
            pl = recarg(p, &idpos);
        } else
            return 0;
    }

    if (mlev == 0)
        lmap_head = idpos;    /* set */
    if (!p->f.flike) {
        lst_flush(mlev, 0);      /* before macro name */
        lst_discard(mlev, 1);    /* removes macro name */
    }
    mcr_eadd(p->chn);
    l = lst_append(l, lex_make(LEX_MCR, p->chn, 0));
    if (pl)
        for (q = p->func.param; *q; q++) {
            r = plookup(pl, *q);
            if (r && r->elist)
                paint(r->elist);
        }
    for (q = p->rlist; *q; ) {
        t = *q++;
#if 0
        if (p->f.sharp && sharp(&q, t, pl, &l))
            continue;
        else
#endif
        if (t->id == LEX_ID) {
            if (pl && (r = plookup(pl, t)) != NULL) {
                l = lst_append(l, lex_make(LEX_MCR, NULL, 0));
                if (r->elist)
                    l = lst_append(l, lst_copyl(r->elist, mlev, strg_line));
                l = lst_append(l, lex_make(LEX_MCR, NULL, 1));
            } else {
                l = lst_append(l, lst_copy(t, mlev, strg_line));
                pe = elookup(t);
                if (EXPANDING(pe))
                    l->f.blue = 1;
            }
        } else
            l = lst_append(l, lst_copy(t, mlev, strg_line));
    }
    l = lst_append(l, lex_make(LEX_MCR, p->chn, 1));
    mcr_edel(p->chn);
    lst_insert(l);
    if (mlev == 0) {
        lmap_head = lmap_head->from;    /* restore */
        assert(lmap_head);
    }

    return 1;
}


/*
 *  (predefined, command-line) initializes macros;
 *  ASSUMPTION: hosted implementation assumed
 */
void (mcr_init)(void)
{
    mtab.t = MEM_CALLOC(MTAB, sizeof(*mtab.t));
    mtab.n = MTAB;
}


/*
 *  frees storages for handling macros
 */
void (mcr_free)(void)
{
    struct emlist *q;

    MEM_FREE(mtab.t);
    for (; em; em = q) {
        q = em->next;
        MEM_FREE(em);
    }
}


#ifndef NDEBUG
/*
 *  (expanding macro list) prints for debugging
 */
void (mcr_eprint)(FILE *fp)
{
    struct emlist *p;

    assert(fp);

    fputs("[ ", fp);
    for (p = em; p; p = p->next)
        if (p->count > 0)
            fprintf(fp, "%s(%d) ", p->chn, p->count);
    fputs("]\n", fp);
}
#endif    /* !NDEBUG */

/* end of mcr.c */
