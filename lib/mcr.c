/*
 *  macro for preprocessing
 */

#include <ctype.h>         /* isdigit */
#include <stddef.h>        /* size_t, NULL */
#include <string.h>        /* memcpy, strcmp, strcpy, strcat, strlen, strncpy */
#include <time.h>          /* time_t, time, ctime */
#include <cbl/arena.h>     /* arena_t, ARENA_ALLOC, ARENA_CALLOC */
#include <cbl/assert.h>    /* assert */
#include <cbl/memory.h>    /* MEM_NEW, MEM_CALLOC, MEM_FREE */
#include <cdsl/hash.h>     /* hash_string, hash_new */
#include <cdsl/list.h>     /* list_t, list_push, list_reverse, list_pop */
#ifndef NDEBUG
#include <stdio.h>         /* FILE, fprintf, fputs */
#endif    /* !NDEBUG */

#include "common.h"
#include "err.h"
#include "in.h"
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

/* helper macro for perm() */
#define swap(i, j) (t=arr[i][0], arr[i][0]=arr[j][0], arr[j][0]=t,    \
                    t=arr[i][1], arr[i][1]=arr[j][1], arr[j][1]=t)

/* (predefined macros) checks if predefined macro */
#define ISPREDMCR(n) ((n)[0] == '_' && (n)[1] == '_')


/* (command list) macros from command line;
   cannot instead use lex_t without strg_init() */
struct cmdl {
    int del;            /* 0: to add, 1: to remove */
    const char *arg;    /* argument string */
};

/* parameter list */
struct pl {
    const char *chn;    /* parameter name (clean, hashed) */
    lex_t **rl;         /* replacement list */
    lex_t *el;          /* expanded replacement list */
    struct pl *next;    /* next entry */
};

/* parameter expansion list */
struct pel {
    const char *chn;     /* parameter name (clean, hashed) */
    const lmap_t *pos;   /* definition locus */
    int expand;          /* # of occurrences for expansion */
    struct pel *next;    /* next entry */
};


/* macro table */
static struct {
    size_t u, n;    /* # of used/total buckets */
    struct mtab {
        const char *chn;      /* macro name (clean, hashed) */
        const lmap_t *pos;    /* definition locus */
        lex_t **rl;           /* replacement list */
        struct {
            unsigned flike:  1;    /* function-like */
            unsigned vaarg:  1;    /* variadic */
            unsigned sharp:  1;    /* has # or ## */
            unsigned predef: 1;    /* predefined macro */
        } f;
        struct {
            int argno;         /* # of arguments */
            lex_t **param;     /* parameters */
            struct pel *pe;    /* parameter expansion list */
        } func;                /* function-like macro */
        struct mtab *link;    /* hash chain */
    } **t;
} mtab;

/* expanding macro list */
static struct eml {
    const char *chn;     /* macro name (clean, hashed) */
    int count;           /* nesting count */
    struct eml *next;    /* next entry */
} *em;

static int diagds;      /* true if issueing ERR_PP_ORDERDS is enabled */
static list_t *cmdl;    /* (command list) macros from command line */
static int nppname;     /* number of macros defined */
static sz_t counter;    /* tracks __COUNTER__ */


/*
 *  (parameter expansion list) adds a parameter
 */
static struct pel *peadd(struct pel *l, const lex_t *t, const lmap_t **found)
{
    const char *chn;
    struct pel *p;

    assert(t);
    assert(found);

    chn = hash_string(LEX_SPELL(t));
    for (p = l; p; p = p->next)
        if (p->chn == chn) {
            *found = p->pos;
            return p;
        }

    p = ARENA_ALLOC(strg_perm, sizeof(*p));
    p->chn = chn;
    p->pos = t->pos;
    p->expand = 0;
    p->next = l;
    *found = NULL;

    return p;
}


/*
 *  (parameter expansion list) looks up a parameter
 */
static struct pel *pelookup(struct pel *p, const lex_t *t)
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
    struct eml *p;

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
static struct eml *elookup(const lex_t *t)
{
    struct eml *p;
    const char *chn;

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
    struct eml **p, *q;

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
static struct pl *padd(struct pl *pl, const lex_t *t, lex_t **rl, lex_t *el)
{
    const char *chn;
    struct pl *p;

    assert(t);

    chn = hash_string(LEX_SPELL(t));
    p = ARENA_ALLOC(strg_perm, sizeof(*p));
    p->chn = chn;
    p->rl = rl;
    p->el = el;
    p->next = pl;

    return p;
}


/*
 *  (parameter list) look up a parameter
 */
static struct pl *plookup(struct pl *p, const lex_t *t)
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
static struct mtab *add(const char *cn, const lmap_t *pos, lex_t *l[], lex_t *param[], int vaarg)
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
            if ((p->f.flike ^ !!param) || p->f.vaarg != vaarg || !eqtlist(p->rl, l) ||
                (param && !eqtlist(p->func.param, param)))
                (void)(err_dpos(pos, ERR_PP_MCRREDEF, chn) &&
                       err_dpos(p->pos, ERR_PARSE_PREVDEF));
            return NULL;
        }
    if (++mtab.u*3 > mtab.n*2 && mtab.n < MAXMT) {
        resize();
        h = hashkey(chn, mtab.n);
    }

    p = ARENA_CALLOC(strg_perm, 1, sizeof(*p));
    p->chn = chn;
    p->pos = pos;
    p->rl = l;
    p->f.flike = !!param;
    p->f.vaarg = vaarg;
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
void (mcr_del)(lex_t *t)
{
    const char *cn;
    struct mtab *p;

    assert(t);

    p = lookup(cn = LEX_SPELL(t));
    MCR_IDVAARGS(cn, t);
    if (p) {
        if (p->f.predef)
            err_dpos(t->pos, ERR_PP_PMCRUNDEF, cn);
        else {
            p->chn = NULL;
            nppname--;
            assert(nppname >= 0);
        }
    } else
        err_dpos(t->pos, ERR_PP_UNDEFMCR, cn);
}


/*
 *  checks if parameters need to be expanded
 */
static void chkexp(struct pel *l, lex_t *p[])
{
    int first = 0;
    struct pel *q;
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
lex_t *(mcr_define)(const lmap_t *pos, int cmd)
{
    int n = -1;
    int sharp = 0;
    lex_t *t, *pt, *v, *l;
    const char *cn, *s;
    const lmap_t *idpos;
    arena_t *strg;
    lex_t **param = NULL;
    struct pel *pe = NULL;

    NEXTSP(t);    /* consumes define */
    if (t->id != LEX_ID) {
        err_dpos(lmap_after(pos), ERR_PP_NOMCRID);
        return t;
    }
    cn = LEX_SPELL(t);
    idpos = t->pos;
    MCR_IDVAARGS(cn, t);
    strg = (mcr_redef(cn))? strg_line: strg_perm;
    t = lst_nexti();
    if (cmd)    /* space allowed before ( */
        SKIPSP(t);

    pt = v = l = NULL;
    if (t->id == '(') {    /* function-like */
        lex_t *pl = NULL;
        const lmap_t *dup, *pos = t->pos;

        n = 0;
        NEXTSP(t);    /* consumes ( */
        while (t->id == LEX_ID || t->id == LEX_ELLIPSIS) {
            pos = t->pos;
            if (n++ == TL_PARAMP_STD)
                (void)(err_dpos(t->pos, ERR_PP_MANYPARAM) &&
                       err_dpos(t->pos, ERR_PP_MANYPSTD, (long)TL_PARAMP_STD));
            if (t->id == LEX_ELLIPSIS) {
                err_dpos(t->pos, ERR_PP_VARIADIC);
                SPELL(t, "__VA_ARGS__");
                t->id = LEX_ID;
                v = t;
            } else {    /* LEX_ID */
                s = LEX_SPELL(t);
                MCR_IDVAARGS(s, t);
            }
            pe = peadd(pe, t, &dup);
            if (dup) {
                err_dmpos(t->pos, ERR_PP_DUPNAME, dup, NULL, pe->chn);
                return t;
            }
            pl = lst_append(pl, lst_copy(t, 0, strg));
            NEXTSP(t);    /* consumes id */
            if (t->id != ',')
                break;
            else if (v) {    /* ..., */
                err_dpos(v->pos, ERR_PP_ELLSEEN);
                return t;
            }
            pos = t->pos;
            NEXTSP(t);    /* consumes , */
            if (t->id != LEX_ID && t->id != LEX_ELLIPSIS) {
                err_dpos(lmap_after(pos), ERR_PP_NOPNAME);
                return t;
            }
        }
        if (t->id != ')') {
            err_dpos(lmap_after(pos), ERR_PP_NOPRPAREN);
            return t;
        }
        param = lst_toarray(pl, strg);
        t = lst_nexti();
    }

    /* replacement list */
    if (t->id != LEX_SPACE && t->id != LEX_NEWLINE && n < 0 && !cmd)
        err_dpos(lmap_after(idpos), ERR_PP_NOSPACE, cn);
    SKIPSP(t);
    if (cmd) {    /* optional = */
        if (t->id == '=')
            NEXTSP(t);    /* consumes = */
        else if (t->id == LEX_EOI) {
            t = lex_make(LEX_PPNUM, "1", 0);
            t->pos = lmap_cmd;
        } else
            err_dpos(t->pos, ERR_PP_NOEQCL, (n >= 0)? "function": "object", cn);
    }
    while (t->id != LEX_NEWLINE && t->id != LEX_EOI) {
        if (t->id == LEX_SPACE) {
            lex_t *u;
            NEXTSP(u);    /* consumes space */
            if (u->id != LEX_NEWLINE && u->id != LEX_EOI) {
                SPELL(t, " ");
                l = lst_append(l, lst_copy(t, 0, strg));
            }
            t = u;
            continue;
        }
        if (t->id == LEX_ID && t->spell[0] == '_' && !v) {    /* before copy */
            s = LEX_SPELL(t);
            MCR_IDVAARGS(s, t);
        }
        l = lst_append(l, lst_copy(t, 0, strg));
        if (n > 0 && t->id == LEX_ID) {
            struct pel *p = pelookup(pe, t);
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
                    l = lst_append(l, lst_copy(t, 0, strg));
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
                } else if (main_opt()->extension && v && pt->id == ',' && t->id == LEX_ID) {
                    s = LEX_SPELL(t);
                    if (MCR_ISVAARGS(s)) {
                        pt->f.vaopt = 1;
                        pt->next = l->next, l = pt;
                        pt = ts;
                        continue;
                    }
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
            pt = ts;
            continue;
        }
        pt = l;    /* not t */
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
        p = add(cn, idpos, lst_toarray(l, strg), param, !!v);
        if (p) {
            if (nppname++ == TL_PPNAME_STD)
                (void)(err_dpos(idpos, ERR_PP_MANYPPID) &&
                       err_dpos(idpos, ERR_PP_MANYPPIDSTD, (long)TL_PPNAME_STD));
            if (n >= 0) {
                p->func.argno = n;
                p->func.pe = pe;
                if (sharp && n > 0)
                    chkexp(pe, p->rl);
            }
            if (sharp)
                p->f.sharp = 1;
            if ((p = conflict(p->chn)) != NULL)
                (void)(err_dpos(idpos, ERR_LEX_LONGID) &&
                       err_dpos(idpos, ERR_LEX_LONGIDSTD, (long)TL_INAME_STD) &&
                       err_dpos(p->pos, ERR_LEX_SEEID, p->chn));
        }
    }

    return t;
}


/*
 *  concatenates tow tokens
 */
const char *concat(const lex_t *t1, const lex_t *t2)
{
    size_t sn;
    char *pbuf;
    const char *s1, *s2;

    assert(t1);
    assert(t2);

    s1 = LEX_SPELL(t1);
    s2 = LEX_SPELL(t2);

    sn = strlen(s1);
    pbuf = snbuf(sn+strlen(s2) + 1, 0);
    strcpy(pbuf, s1);
    strcat(pbuf+sn, s2);

    return pbuf;
}


/*
 *  permutates tokens to check validity of successive ##'s
 */
static lex_t *perm(lex_t *arr[][2], int l, int u)
{
    static lex_t *ta[MAXDS][2];

    int i;
    lex_t *t, *gl;

    assert(arr);
    assert(u <= MAXDS);

    if (l == u) {
        memcpy(ta, arr, sizeof(ta));
        for (i = 0; i < u; ) {
            lex_t **pp, *p = ta[i][0], *n = ta[i][1];
            const char *buf = concat(p, n);
            gl = lst_run(buf, n->pos);
            if (!gl) {
                gl = lex_make(LEX_SPACE, "", 0);
                gl->pos = n->pos;
            } else if (gl->next != gl) {
                diagds = 0;
                gl->spell = buf;
                return gl;
            }
            for (pp = (lex_t **)&ta[++i]; pp < (lex_t **)&ta[u]; pp++)
                if (*pp == p || *pp == n)
                    *pp = gl;
        }
        return NULL;
    }

    for (i = l; i < u; i++) {
        if (i != l)
            swap(l, i);
        if ((gl = perm(arr, l+1, u)) != NULL)
            break;
        if (i != l)
            swap(l, i);
    }

    return gl;
}


/*
 *  paints tokens being expanded in an expanded replacement list or ##
 */
static void paint(lex_t *l)
{
    lex_t *t;
    struct eml *pe;

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
 *  checks if the evaluation order of ## affects program validity
 */
static lex_t *deporder(lex_t *l)
{
    static lex_t *arr[MAXDS][2];

    int i, n;
    lex_t *gl;

    if (!l || (n = lst_length(l)) == 2)
        return NULL;

    if (n > MAXDS+1) {
        for (i=0, l=l->next; i < n-1; i++, l=l->next) {
            const char *buf = concat(l, l->next);
            gl = lst_run(buf, l->next->pos);
            if (gl && gl->next != gl) {
                gl->spell = buf;
                return gl;
            }
        }
        return NULL;
    }

    for (i=0, l=l->next; i < n-1; i++, l=l->next) {
        arr[i][0] = l;
        arr[i][1] = l->next;
    }

    return perm(arr, 0, n-1);
}


/*
 *  concatenates two tokens
 */
static lex_t *paste(lex_t *t1, lex_t *t2, struct pl *pl, lex_t **ll, lex_t **pdsl,
                    const lmap_t *dpos, const lmap_t *tpos)
{
    static lex_t empty = {
        LEX_SPACE,
        "",
        NULL,
        { 0, 1, 0, 0, 0, 0 },
        &empty
    };

    lex_t *l;
    struct pl *p;
    lex_t **q = NULL;
    const char *buf;
    lex_t *gl, *r;

    assert(t1);
    assert(t2);
    assert(ll);
    assert(pdsl);

    l = *ll;
    if (pl) {
        if (t1->id == LEX_ID && !t1->f.noarg && (p = plookup(pl, t1)) != NULL) {
            if (*(q = p->rl) != NULL) {
                for (; q[1]; q++)
                    l = lst_append(l, lst_copy(*q, 1, strg_line));
                t1 = *q;
            } else
                t1 = &empty;
        }
        q = NULL;
        if (t2->id == LEX_ID && (p = plookup(pl, t2)) != NULL) {
            if ((t2 = p->rl[0]) != NULL)
                q = p->rl + 1;
            else
                t2 = &empty;
        }
    }
    assert(t1);
    assert(t2);

    if (diagds) {
        if (!*pdsl)
            *pdsl = lst_append(*pdsl, lst_copy(t1, 0, strg_line));
        r = lst_copy(t2, 0, strg_line);
        r->pos = dpos;
        *pdsl = lst_append(*pdsl, r);
    }

    buf = concat(t1, t2);
    gl = lst_run(buf, tpos);
    if (!gl) {
        err_dpos(dpos, ERR_PP_EMPTYTOKMADE);
        *ll = l;
        return &empty;
    } else if (gl->next != gl) {
        err_dpos(dpos, ERR_PP_INVTOKMADE, buf);
        diagds = 0;
    }
    paint(gl);
    if (diagds && q && *q) {
        if ((r = deporder(*pdsl)) != NULL) {
            (void)(err_dpos(r->pos, ERR_PP_ORDERDS) &&
                   err_dpos(r->pos, ERR_PP_ORDERDSEX, r->spell));    /* clean */
            diagds = 0;
        }
        *pdsl = NULL;
    }

    r = gl->next;    /* r from gl has correct u->m chain */
    for (r = gl->next; r != gl; r = r->next)
        l = lst_append(l, lst_copy(r, 0, strg_line));
    if (q && *q) {
        l = lst_append(l, lst_copy(r, 0, strg_line));
        while (q[1])
            l = lst_append(l, lst_copy(*q++, 1, strg_line));
        t1 = lst_copy(*q, 1, strg_line);
    } else
        t1 = lst_copy(r, 0, strg_line);

    t1->f.noarg = 1;
    *ll = l;
    return t1;
}


/*
 *  looks for the next non-space token in a token array
 */
static lex_t **nextnsp(lex_t *pp[])
{
    assert(pp);

    if (*pp)
        while ((*pp)->id == LEX_SPACE)
            pp++;

    return pp;
}


/*
 *  stringifies a macro parameter
 */
static lex_t *stringify(lex_t ***pq, struct pl *pl, const lmap_t *dpos, const lmap_t *tpos)
{
    int size = 20;
    struct pl *p;
    char *buf, *pb;
    lex_t **r, *t;

    assert(pq);
    assert(*pq);
    assert(**pq);
    assert(pl);

    *pq = nextnsp(*pq);
    assert((**pq)->id == LEX_ID);
    p = plookup(pl, **pq);
    pb = buf = ARENA_ALLOC(strg_line, sizeof(*buf) * size);
    *pb++ = '"';
    if (p)
        for (r = p->rl; *r; r++) {
            const char *s = LEX_SPELL(*r);
            while (*s) {
                if (((*r)->id == LEX_SCON || (*r)->id == LEX_CCON) && (*s == '"' || *s == '\\'))
                    *pb++ = '\\';
                *pb++ = *s++;
                if (pb > buf + size - 2) {
                    char *pold = buf;
                    buf = ARENA_ALLOC(strg_line, sizeof(*buf) * (size+=20));
                    memcpy(buf, pold, pb - pold);
                    pb = buf + (pb - pold);
                }
            }
        }
    *pb++ = '"';
    *pb = '\0';
    (*pq)++;

    for (pb = buf+1; *pb && *pb != '"'; pb++)
        if (*pb == '\\')
            pb++;    /* cannot be NUL */

    t = lex_make(LEX_SCON, buf, 0);
    t->pos = tpos;

    if (!(pb[0] == '"' && pb[1] == '\0'))
        err_dpos(lmap_macro(dpos, lmap_from, strg_line), ERR_PP_INVSTRMADE, buf);

    return t;
}


/*
 *  handles # and ## operators
 */
int sharp(lex_t ***pq, lex_t *t1, struct pl *pl, lex_t **ll)
{
    int nend = 0;
    lex_t *l, *t2;
    lex_t *dsl = NULL;
    const lmap_t *spos, *ppos,    /* # and ## */
                 *fpos, *lpos,    /* first and last token of range */
                 *tpos;           /* placeholder for token position */

    assert(pq);
    assert(t1);
    assert(ll);

    spos = ppos = NULL;
    diagds = err_chkwarn(ERR_PP_ORDERDS);

    l = *ll;
    fpos = t1->pos;
    tpos = lmap_macro(t1->pos, lmap_from, strg_line);    /* u.m will be overridden */
    while (1) {
        if (t1->id == LEX_STROP && pl) {
            assert(!nend);
            l = lst_append(l, lex_make(LEX_MCR, NULL, 0));
            nend = 1;
            spos = t1->pos;
            t1 = stringify(pq, pl, spos, tpos);
            lpos = (*pq)[-1]->pos;
            continue;
        } else if (t1->id != LEX_SPACE || isempty(t1)) {
            lex_t **r = nextnsp(*pq);
            if (*r && (*r)->id == LEX_PASTEOP) {
                *pq = nextnsp(r+1) + 1;
                t2 = (*pq)[-1];
                if (!nend) {
                    l = lst_append(l, lex_make(LEX_MCR, NULL, 0));
                    nend = 1;
                }
                if (t2->id == LEX_STROP && pl) {
                    spos = t2->pos;
                    t2 = stringify(pq, pl, spos, tpos);
                }
                lpos = (*pq)[-1]->pos;
                ppos = lmap_macro((*r)->pos, lmap_from, strg_line);
                t1 = paste(t1, t2, pl, &l, &dsl, ppos, tpos);
                continue;
            }
        }
        break;
    }
    if (diagds && dsl && (t2 = deporder(dsl)) != NULL)
        (void)(err_dpos(t2->pos, ERR_PP_ORDERDS) &&
               err_dpos(t2->pos, ERR_PP_ORDERDSEX, t2->spell));    /* clean */
    if (spos && ppos)
        err_dmpos(ppos, ERR_PP_ORDERSDS, spos, NULL);

    if (nend) {
        if (!isempty(t1)) {    /* t1 already has correct u->m chain */
            ((lmap_t *)tpos)->u.m = lmap_range(fpos, lpos);
            l = lst_append(l, lst_copy(t1, 0, strg_line));
        }
        l = lst_append(l, lex_make(LEX_MCR, NULL, 1));
    }

    *ll = l;
    return nend;
}


/*
 *  expands macros from arguments
 */
static lex_t *exparg(lex_t *l, const lmap_t *pos)
{
    lex_t *t;
    const lmap_t *tpos;

    if (!l)
        return NULL;

    lst_push(l);
    tpos = lmap_from, lmap_from = pos;    /* set */

    while ((t = lst_nexti())->id != LEX_EOI) {
        assert(t->id != LEX_NEWLINE);
        if (t->id == LEX_ID)
            mcr_expand(t);
    }
    lst_flush(1);

    lmap_from = tpos;    /* restore */
    return lst_pop();
}


#define ISNL(nl) (t->id == LEX_NEWLINE && ((nl)=t, !lex_direc))

/*
 *  skips spaces and newlines in macro arguments
 */
static lex_t *nextspnl(int nl, lex_t **pnl)
{
    lex_t *t;

    assert(pnl);

    NEXTSP(t);
    if (ISNL(*pnl)) {
        nl = 1;
        while ((t = lst_nexti())->id == LEX_SPACE || ISNL(*pnl))
            continue;
    }
    if (nl && t->id == LEX_SHARP)
        err_dpos(t->pos, ERR_PP_DIRECINARG);

    return t;
}


/*
 *  recognizes arguments for function-like macros
 */
static struct pl *recarg(struct mtab *p, const lmap_t **ppos, int *pnoarg)
{
    int errarg = 0;
    int level = 1, n = 0;
    lex_t *t, *nl = NULL;
    lex_t *tl = NULL, **rl;
    struct pl *pl = NULL;
    const lmap_t *pos, *prnpos, *tpos;

    assert(p);
    assert(ppos);

    pos = *ppos;
    while ((t = lst_nexti())->id != '(')
        continue;
    prnpos = t->pos;
    t = nextspnl(0, &nl);    /* consumes ( */

    while (t->id != LEX_EOI && (t->id != LEX_NEWLINE || (nl=t, !lex_direc))) {
        if (t->id == LEX_SPACE || ISNL(nl)) {
            lex_t *u = nextspnl(t->id == LEX_NEWLINE, &nl);    /* consumes space or newline */
            if (!(level == 1 && (u->id == ',' || u->id == ')')) && u->id != LEX_EOI &&
                (!lex_direc || u->id != LEX_NEWLINE)) {
                t->id = LEX_SPACE;
                SPELL(t, " ");
                tl = lst_append(tl, lst_copy(t, 0, strg_line));
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
                    tl = lst_append(tl, lst_copy(t, 0, strg_line));
                } else if (p->f.vaarg && level == 1 && n == p->func.argno - !tl) {
                    if (!tl && ++n == TL_ARGP_STD+1 && !errarg) {
                        tpos = lmap_macro(t->pos, pos, strg_line);
                        (void)(err_dpos(tpos, ERR_PP_MANYARGW, p->chn) &&
                               err_dpos(tpos, ERR_PP_MANYARGSTD, (long)TL_ARGP_STD));
                    }
                    tl = lst_append(tl, lst_copy(t, 0, strg_line));
                } else {
                    if (t->id == ',' || p->func.argno > 0) {
                        if (!tl) {
                            if (n++ == p->func.argno) {
                                ((void)(err_dpos(lmap_macro(t->pos, pos, strg_line),
                                                 ERR_PP_MANYARG, p->chn) &&
                                        err_dpos(p->pos, ERR_PARSE_DEFHERE)));
                                errarg = 1;
                            } else if ((t->id == ',' && n <= p->func.argno) ||
                                       n == p->func.argno) {
                                if (p->func.argno == 1)
                                    *pnoarg = 1;
                                err_dpos(lmap_macro(t->pos, pos, strg_line), ERR_PP_EMPTYARG,
                                         p->chn);
                            }
                            if (n == TL_ARGP_STD+1 && !errarg) {
                                tpos = lmap_macro(t->pos, pos, strg_line);
                                (void)(err_dpos(tpos, ERR_PP_MANYARGW, p->chn) &&
                                       err_dpos(tpos, ERR_PP_MANYARGSTD, (long)TL_ARGP_STD));
                            }
                        }
                        if (n <= p->func.argno) {
                            struct pel *pe;
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
                    t = nextspnl(0, &nl);    /* consumes , */
                    continue;
                }
                break;
            case '(':
                level++;
            default:
                if (!tl) {
                    if (n++ == p->func.argno) {
                        ((void)(err_dpos(lmap_macro(t->pos, pos, strg_line), ERR_PP_MANYARG,
                                         p->chn) &&
                                err_dpos(p->pos, ERR_PARSE_DEFHERE)));
                        errarg = 1;
                    }
                    if (n == TL_ARGP_STD+1 && !errarg) {
                        tpos = lmap_macro(t->pos, pos, strg_line);
                        (void)(err_dpos(tpos, ERR_PP_MANYARGW, p->chn) &&
                               err_dpos(tpos, ERR_PP_MANYARGSTD, (long)TL_ARGP_STD));
                    }
                }
                tl = lst_append(tl, lst_copy(t, 0, strg_line));
                break;
        }
        t = lst_nexti();
    }

    ret:
        if (t->pos && pos->from == t->pos->from)    /* same nesting level */
            *ppos = lmap_range(*ppos, t->pos);
        if (level > 0)
            err_dpos(lmap_macro(prnpos, pos, strg_line), ERR_PP_UNTERMARG, p->chn);
        else if (n < p->func.argno) {
            const lmap_t *tpos = lmap_macro(t->pos, pos, strg_line);
            if (p->f.vaarg && n == p->func.argno-1)
                ((void)(err_dpos(tpos, ERR_PP_ARGTOVAARGS, p->chn) &&
                        err_dpos(p->pos, ERR_PARSE_DEFHERE)));
            else
                ((void)(err_dpos(tpos, ERR_PP_INSUFFARG, p->chn) &&
                        err_dpos(p->pos, ERR_PARSE_DEFHERE)));
            *pnoarg = 1;
            while (n++ < p->func.argno)
                pl = padd(pl, p->func.param[n-1], lst_toarray(tl, strg_line), NULL);
        }

        if ((t->id == LEX_EOI || t->id == LEX_NEWLINE) && nl)
            lst_insert(lst_copy(nl, 0, strg_line));
        lst_discard(1);    /* removes from macro name to end of invocation */

        return pl;
}

#undef ISNL


/*
 *  makes a string literal representation for tokens
 */
static const char *mkstr(const char *s, arena_t *a)
{
    size_t len;
    char *buf, *p;

    assert(s);
    assert(*s != '"');
    assert(a);

    len = strlen(s)*2 + 2 + 1;
    p = buf = ARENA_ALLOC(a, len);
    *p++ = '"';
    while (*s) {
        if (*s == '"' || *s == '\\')
            *p++ = '\\';
        *p++ = *s++;
    }
    *p++ = '"';
    *p = '\0';

    return buf;
}


/*
 *  expands an identifier if it denotes a macro
 */
int (mcr_expand)(lex_t *t)
{
    int noarg;
    const char *s;
    struct mtab *p;
    struct eml *pe;
    struct pl *pl = NULL, *r;
    lex_t *l = NULL, **q;
    const lmap_t *idpos, *tpos;

    assert(t);

    p = lookup(s = LEX_SPELL(t));
    MCR_IDVAARGS(s, t);
    if (!p || t->f.blue)
        return 0;

    {    /* handles predefined macros */
        const lmap_t *q;

        if (p->f.predef) {
            assert(ISPREDMCR(s));
            switch(s[2]) {
                case 'F':
                    if (strcmp(s, "__FILE__") == 0) {
                        assert(!p->rl[1]);
                        q = lmap_nfrom(lmap_from);
                        p->rl[0]->spell = mkstr(q->u.i.f, strg_line);    /* cis */
                    }
                    break;
                case 'L':
                    if (strcmp(s, "__LINE__") == 0) {
                        assert(!p->rl[1]);
                        q = lmap_nfrom(lmap_from);
                        s = ARENA_ALLOC(strg_line, STRG_BUFN + 1);
                        sprintf((char *)s, "%"FMTSZ"u", in_py+q->u.i.yoff);    /* cis */
                        p->rl[0]->spell = s;
                    }
                    break;
                case 'C':
                    if (strcmp(s, "__COUNTER__") == 0) {
                        assert(!p->rl[1]);
                        s = ARENA_ALLOC(strg_line, STRG_BUFN + 1);
                        sprintf((char *)s, "%"FMTSZ"u", counter++);
                        p->rl[0]->spell = s;
                    }
                    break;
            }
        }
    }

    idpos = t->pos;
    /* lmap_from not set here to adjust idpos in recarg() */
    if (p->f.flike) {
        lex_t *u = lst_peeki();
        if (u->id == '(') {
            ((lex_direc)? lst_discard: lst_flush)(0);    /* before macro name */
            noarg = 0;
            pl = recarg(p, &idpos, &noarg);
        } else
            return 0;
    } else {
        if (!lex_direc)
            lst_flush(0);    /* before macro name */
        lst_discard(1);      /* removes macro name */
    }

    tpos = lmap_from, lmap_from = idpos;    /* set */
    mcr_eadd(p->chn);
    l = lst_append(l, lex_make(LEX_MCR, p->chn, 0));
    if (pl)
        for (q = p->func.param; *q; q++) {
            r = plookup(pl, *q);
            if (r && r->el)
                paint(r->el);
        }
    for (q = p->rl; *q; ) {
        t = *q++;
        if (t->f.vaopt && noarg)
            continue;
        if (p->f.sharp && sharp(&q, t, pl, &l))
            continue;
        else if (t->id == LEX_ID) {
            if (pl && (r = plookup(pl, t)) != NULL) {
                l = lst_append(l, lex_make(LEX_MCR, NULL, 0));
                if (r->el)    /* el already has correct u.m chain */
                    l = lst_append(l, lst_copyl(r->el, 0, strg_line));
                l = lst_append(l, lex_make(LEX_MCR, NULL, 1));
            } else {
                l = lst_append(l, lst_copy(t, 1, strg_line));
                pe = elookup(t);
                if (EXPANDING(pe))
                    l->f.blue = 1;
            }
        } else
            l = lst_append(l, lst_copy(t, 1, strg_line));
    }
    l = lst_append(l, lex_make(LEX_MCR, p->chn, 1));
    mcr_edel(p->chn);
    lst_insert(l);
    lmap_from = tpos;    /* restore */

    return 1;
}


/*
 *  (predefined macro) adds a macro into the macro table
 */
static void addpr(const char *name, int tid, const char *val)
{
    lex_t *t = NULL;
    struct mtab *p;

    assert(name);
    assert(tid == LEX_SCON || tid == LEX_PPNUM);
    assert(val);
    assert(tid != LEX_PPNUM || isdigit(*(unsigned char *)val));
    assert(!mcr_redef(name));
    assert(ISPREDMCR(name));

    if (*val) {
        if (tid == LEX_SCON && *val != '"')
            val = mkstr(val, strg_perm);
        t = lst_copy(lex_make(tid, val, 0), 0, strg_perm);
        t->pos = lmap_bltin;
    }

    p = add(name, lmap_bltin, lst_toarray(t, strg_perm), NULL, 0);
    assert(p);
    p->f.predef = 1;
}


/*
 *  (command line) adds or removes macro definitions to parse later
 */
void (mcr_cmd)(int del, const char *arg)
{
    struct cmdl *p;

    assert(arg);

    MEM_NEW(p);
    p->del = del;
    p->arg = arg;
    cmdl = list_push(cmdl, p);
}


/*
 *  (predefined, command-line) initializes macros;
 *  ASSUMPTION: hosted implementation assumed
 */
void (mcr_init)(void)
{
    static char pdate[] = "\"May  4 1979\"",
                ptime[] = "\"07:10:05\"";

    time_t tm = time(NULL);
    char *p = ctime(&tm);    /* Fri May  4 07:10:05 1979\n */

    mtab.t = MEM_CALLOC(MTAB, sizeof(*mtab.t));
    mtab.n = MTAB;

    /* __DATE__ */
    strncpy(pdate+1, p+4, 7);
    strncpy(pdate+8, p+20, 4);
    addpr("__DATE__", LEX_SCON, pdate);

    /* __TIME__ */
    strncpy(ptime+1, p+11, 8);
    addpr("__TIME__", LEX_SCON, ptime);

    /* __FILE__ */
    addpr("__FILE__", LEX_SCON, "\"\"");    /* to be generated dynamically */

    /* __LINE__ */
    addpr("__LINE__", LEX_PPNUM, "0");    /* to be generated dynamically */

    /* __STDC__, __STDC_HOSTED__, __STDC_VERSION__ */
    if (main_opt.std) {
        addpr("__STDC__", LEX_PPNUM, "1");
        addpr("__STDC_HOSTED__", LEX_PPNUM, "1");
        addpr("__STDC_VERSION__", LEX_PPNUM, TL_VER_STD);
    }

    /* __CHAR_UNSIGNED__ */
    if (main_opt.uchar)
        addpr("__CHAR_UNSIGNED__", LEX_PPNUM, "1");

    /* __COUNTER__ */
    addpr("__COUNTER__", LEX_PPNUM, "0");    /* to be generated dynamically */

    cmdl = list_reverse(cmdl);
    while (cmdl) {
        void *c;
        lex_t *t;
        const char *name;

        cmdl = list_pop(cmdl, &c);
        if (*((struct cmdl *)c)->arg == '\0')    /* e.g., -D= */
            err_dpos(lmap_cmd, ERR_PP_NOMCRID);
        else {
            lst_push(lst_run(((struct cmdl *)c)->arg, lmap_cmd));
            if (!((struct cmdl *)c)->del)
                mcr_define(lmap_cmd, 1);
            else {
                NEXTSP(t);    /* first token */
                if (t->id != LEX_ID)
                    err_dpos(lmap_cmd, ERR_PP_NOMCRID);
                else {
                    name = LEX_SPELL(t);
                    mcr_del(t);
                    NEXTSP(t);    /* consumes id */
                    if (t->id != LEX_EOI)
                        err_dpos(lmap_cmd, ERR_PP_EXTRATOKENCL, name);
                }
            }
            lst_pop();
        }
        MEM_FREE(c);
    }

}


/*
 *  frees storages for handling macros
 */
void (mcr_free)(void)
{
    struct eml *q;

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
    struct eml *p;

    assert(fp);

    fputs("[ ", fp);
    for (p = em; p; p = p->next)
        if (p->count > 0)
            fprintf(fp, "%s(%d) ", p->chn, p->count);
    fputs("]\n", fp);
}
#endif    /* !NDEBUG */

/* end of mcr.c */
