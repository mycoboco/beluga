/*
 *  macro for preprocessing
 */

#include <ctype.h>         /* isdigit */
#include <stddef.h>        /* size_t, NULL */
#include <stdio.h>         /* sprintf */
#include <string.h>        /* memcpy, strcmp, strcpy, strcat, strlen, strncpy */
#include <time.h>          /* time_t, time, ctime */
#include <cbl/arena.h>     /* arena_t, ARENA_ALLOC, ARENA_CALLOC */
#include <cbl/assert.h>    /* assert */
#include <cbl/memory.h>    /* MEM_NEW, MEM_CALLOC, MEM_FREE */
#include <cdsl/hash.h>     /* hash_string, hash_new */
#include <cdsl/list.h>     /* list_t, list_push, list_reverse, list_pop */
#ifndef NDEBUG
#include <stdio.h>         /* FILE, fprintf, putc, fputs */
#endif    /* !NDEBUG */

#include "../src/alist.h"
#include "../src/common.h"
#include "../src/err.h"
#include "../src/in.h"
#include "../src/main.h"
#include "ctx.h"
#include "lex.h"
#include "lxl.h"
#include "strg.h"
#include "util.h"
#include "mcr.h"

#define MTAB  128      /* initial size of macro table; must be power of 2 */
#define MAXMT 32768    /* max size of macro table */

#define MAXDS 6        /* max number of successive #'s to fully diagnose */

#define EXPANDING(p) ((p) && (p)->count > 0)    /* checks if macro being expanded */
#define isempty(t)   (*(t)->rep == '\0')        /* true if space token denotes empty token */
#define T(p)         ((lex_t *)p)               /* shorthand for cast to lex_t * */

/* helper macro for perm() */
#define swap(i, j) (t=arr[i][0], arr[i][0]=arr[j][0], arr[j][0]=t,    \
                    t=arr[i][1], arr[i][1]=arr[j][1], arr[j][1]=t)

/* (predefined macros) checks if predefined macro */
#define ISPREDMCR(n) ((n)[0] == '_' && (n)[1] == '_')


typedef void *node_t;    /* refers to void * for readability */

/* (command list) macros from command line */
struct cmdlist {
    int del;            /* 0: to add, 1: to remove */
    const char *arg;    /* argument string */
};

/* parameter list */
struct plist {
    const char *name;      /* parameter name */
    node_t *rlist;         /* (lex_t) replacement list */
    lxl_t *elist;          /* expanded replacement list */
    struct plist *next;    /* next entry */
};

/* parameter expansion list */
struct pelist {
    const char *name;       /* parameter name */
    int expand;             /* # of occurrences for expansion */
    struct pelist *next;    /* next entry */
};


const lex_pos_t *mcr_mpos;    /* locus for diagnostics */


/* macro table */
static struct {
    size_t u, n;    /* # of used/total buckets */
    struct mtab {
        const char *name;    /* macro name */
        lex_pos_t pos;       /* definition locus */
        node_t *rlist;       /* (lex_t) replacement list */
        struct {
            unsigned flike:  1;    /* function-like */
            unsigned sharp:  1;    /* has # or ## */
            unsigned predef: 1;    /* predefined macro */
        } f;
        struct {
            int argno;            /* # of arguments */
            node_t *param;        /* (lex_t) parameters */
            struct pelist *pe;    /* parameter expansion list */
        } func;                   /* function-like macro */
        struct mtab *link;    /* hash chain */
    } **t;
} mtab;

/* expanding macro list */
static struct emlist {
    const char *name;       /* macro name */
    int count;              /* nesting count */
    unsigned metend: 1;     /* true when LXL_KEND encountered */
    struct emlist *next;    /* next entry */
} *em;

static const lex_pos_t *apos;    /* locus for diagnostics */
static int diagds;               /* true if issueing ERR_PP_ORDERDS is enabled */
static list_t *cmdlist;          /* (command list) macros from command line */
static node_t *cltok;            /* (lex_t) list from which nextcl() brings tokens */
static int nppname;              /* number of macros defined */


/*
 *  (parameter expansion list) adds a parameter
 */
static struct pelist *peadd(struct pelist *list, const char *name, int *found)
{
    struct pelist *p;

    assert(name);
    assert(found);

    name = hash_string(name);
    for (p = list; p; p = p->next)
        if (p->name == name) {
            *found = 1;
            return list;
        }

    p = ARENA_ALLOC(*strg_tok, sizeof(*p));
    p->name = name;
    p->expand = 0;
    p->next = list;
    *found = 0;

    return p;
}


/*
 *  (parameter expansion list) looks up a parameter
 */
static struct pelist *pelookup(struct pelist *p, const char *name)
{
    assert(name);

    name = hash_string(name);
    for (; p; p = p->next)
        if (p->name == name)
            break;

    return p;
}


/*
 *  (expanding macro list) adds an identifier
 */
void (mcr_eadd)(const char *name)
{
    struct emlist *p;

    assert(name);

    name = hash_string(name);
    for (p = em; p; p = p->next)
        if (p->name == name)
            break;

    if (!p) {
        MEM_NEW(p);
        p->name = name;
        p->count = 0;
        p->next = em;
    }
    p->count++;
    p->metend = 0;
    em = p;
}


/*
 *  (expanding macro list) looks up an identifier
 */
static struct emlist *elookup(const char *name)
{
    struct emlist *p;

    assert(name);

    name = hash_string(name);
    for (p = em; p; p = p->next)
        if (p->name == name)
            break;

    return p;
}


/*
 *  (expanding macro list) removes an identifier
 */
void (mcr_edel)(const char *name)
{
    struct emlist **p, *q;

    assert(name);

    name = hash_string(name);
    for (p = &em; *p; p = &(*p)->next)
        if ((*p)->name == name && --(*p)->count == 0) {
            q = *p;
            *p = q->next;
            MEM_FREE(q);
            break;
        }
}


/*
 *  (expanding macro list) sets a flag when tokens are taken across LXL_KEND
 */
void (mcr_emeet)(const char *name)
{
    struct emlist *p = elookup(name);

    if (p && p->count > 0)
        p->metend = 1;
}


/*
 *  (parameter list) adds a macro parameter
 */
static struct plist *padd(struct plist *pl, const char *name, const alist_t *rlist, lxl_t *elist)
{
    struct plist *p;

    assert(name);

    name = hash_string(name);
    p = ARENA_ALLOC(strg_line, sizeof(*p));
    p->name = name;
    p->rlist = alist_toarray(rlist, strg_line);
    p->elist = elist;
    p->next = pl;

    return p;
}


/*
 *  (parameter list) looks up a parameter
 */
static struct plist *plookup(struct plist *p, const char *name)
{
    assert(name);

    name = hash_string(name);
    for (; p; p = p->next)
        if (p->name == name)
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
            h = hashkey(p->name, nn);
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
static int eqtlist(node_t p[], node_t q[])    /* lex_t */
{
    assert(p);
    assert(q);

    while (*p && *q && T(*p)->id == T(*q)->id && strcmp(T(*p)->rep, T(*q)->rep) == 0)
        p++, q++;

    return (!*p && !*q);
}


/*
 *  (macro table) adds an identifier
 */
static struct mtab *add(const char *name, const lex_pos_t *ppos,
                        node_t list[], node_t param[])     /* lex_t */
{
    unsigned h;
    struct mtab *p;

    assert(name);
    assert(ppos);

    name = hash_string(name);
    h = hashkey(name, mtab.n);
    for (p = mtab.t[h]; p; p = p->link)
        if (p->name == name) {
            if ((p->f.flike ^ !!param) || !eqtlist(p->rlist, list) ||
                (param && !eqtlist(p->func.param, param)))
                err_issuep(ppos, ERR_PP_MCRREDEF, name, &p->pos);
            return NULL;
        }
    if (++mtab.u*3 > mtab.n*2 && mtab.n < MAXMT) {
        resize();
        h = hashkey(name, mtab.n);
    }

    p = ARENA_CALLOC(strg_perm, 1, sizeof(*p));
    p->name = name;
    p->pos = *ppos;
    p->rlist = list;
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
static struct mtab *lookup(const char *name)
{
    unsigned h;
    struct mtab *p;

    assert(name);

    name = hash_string(name);
    h = hashkey(name, mtab.n);
    for (p = mtab.t[h]; p; p = p->link)
        if (p->name == name)
            break;

    return p;
}


/*
 *  (macro table) checks if an identifier has been #defined
 */
int (mcr_redef)(const char *name)
{
    return !!lookup(name);
}


/*
 *  (macro table) #undefines a macro
 */
void (mcr_del)(const char *name, const lex_pos_t *ppos)
{
    struct mtab *p = lookup(name);

    assert(ppos);

    if (p) {
        if (p->f.predef)
            err_issuep(ppos, ERR_PP_PMCRUNDEF, name);
        else {
            p->name = NULL;
            nppname--;
            assert(nppname >= 0);
        }
    } else
        err_issuep(ppos, ERR_PP_UNDEFMCR, name);
}


/*
 *  checks if parameters need to be expanded
 */
static void chkexp(struct pelist *list, node_t p[])
{
    int first = 0;
    struct pelist *q;
    lex_t *t, *pt = NULL;

    assert(list);
    assert(p);

    while (*p) {
        t = *p;
        switch(t->id) {
            case LEX_ID:
                if (pt) {
                    if (pt->id == LEX_STROP) {
                        pelookup(list, t->rep)->expand--;
                        t = NULL;
                        break;
                    } else if (pt->id == LEX_PASTEOP) {
                        if ((q = pelookup(list, t->rep)) != NULL)
                            q->expand--;
                    } else
                        first = 0;
                }
                break;
            case LEX_PASTEOP:
                if (!first) {
                    first = 1;
                    if (pt && pt->id == LEX_ID && (q = pelookup(list, pt->rep)) != NULL)
                        q->expand--;
                }
                break;
            default:
                first = 0;
                break;
        }
        pt = t;
        while (*++p && T(*p)->id == LEX_SPACE)
            continue;
    }
}


/*
 *  detects macro name conflicts
 */
static struct mtab *conflict(const char *name)
{
    static struct ctab {
        const char *cname;
        const char *name;
        struct ctab *link;
    } *ctab[16];

    unsigned h;
    const char *cname;
    struct ctab *p, *q;
    struct mtab *r;

    assert(name);

    if (!main_opt()->std || snlen(name, TL_INAME_STD) < TL_INAME_STD)
        return NULL;

    cname = hash_new(name, TL_INAME_STD);
    h = hashkey(cname, NELEM(ctab));
    q = NULL, r = NULL;
    for (p = ctab[h]; p; p = p->link)
        if (cname == p->cname) {
            if (name == p->name)
                q = p;
            else if (!r)
                r = lookup(p->name);
            if (r && q)
                return r;
        }

    if (!q) {
        p = ARENA_ALLOC(strg_perm, sizeof(*p));
        p->cname = cname;
        p->name = name;
        p->link = ctab[h];
        ctab[h] = p;
    }

    return r;
}


/*
 *  accepts and handles macro definitions;
 *  does what should be done in proc.c for command-line definitions
 */
lex_t *(mcr_define)(int cmd, lex_t *(*next)(void), const lex_pos_t *ppos)
{
    static lex_t one = {
        LEX_PPNUM,
        0,
        "1"
    };

    int n = -1;
    int sharp = 0;
    lex_t *t;
    const char *name;
    lex_pos_t idpos;
    node_t *param = NULL;    /* lex_t */
    alist_t *list = NULL;
    struct pelist *pe = NULL;

    assert(next);
    assert(ppos);

    t = skip(NULL, next);
    if (t->id != LEX_ID) {
        err_issuep(ppos, ERR_PP_NOMCRID);
        return t;
    }
    name = t->rep;
    idpos = *ppos;
    if (!mcr_redef(name))
        strg_tok = &strg_perm;
    t = next();
    if (cmd)    /* space allowed before ( */
        t = skip(t, next);

    if (t->id == '(') {    /* function-like */
        int dup;
        alist_t *plist = NULL;

        n = 0;
        t = skip(NULL, next);
        while (t->id == LEX_ID) {
            if (n++ == TL_PARAMP_STD) {
                err_issuep(ppos, ERR_PP_MANYPARAM);
                err_issuep(ppos, ERR_PP_MANYPSTD, (long)TL_PARAMP_STD);
            }
            pe = peadd(pe, t->rep, &dup);
            if (dup) {
                err_issuep(ppos, ERR_PP_DUPNAME, t->rep);
                return t;
            }
            plist = alist_append(plist, t, strg_line);
            t = skip(NULL, next);
            if (t->id != ',')
                break;
            t = skip(NULL, next);
            if (t->id != LEX_ID) {
                err_issuep(ppos, ERR_PP_NOPNAME);
                return t;
            }
        }
        if (t->id != ')') {
            err_issuep(ppos, ERR_PP_NOPRPAREN);
            return t;
        }
        param = alist_toarray(plist, *strg_tok);
        t = next();
    }

    /* replacement list */
    if (t->id != LEX_SPACE && t->id != LEX_NEWLINE && n < 0 && !cmd)
        err_issuep(ppos, ERR_PP_NOSPACE, name);
    t = skip(t, next);
    if (cmd) {    /* optional = */
        if (t->id == '=')
            t = skip(NULL, next);
        else if (t->id == LEX_EOI)
            t = &one;
        else
            err_issuep(ppos, ERR_PP_NOEQCL, (n >= 0)? "function": "object", name);
    }
    while (t->id != LEX_NEWLINE && t->id != LEX_EOI) {
        if (t->id == LEX_SPACE) {
            lex_t *u = skip(NULL, next);
            if (u->id != LEX_NEWLINE && u->id != LEX_EOI) {
                t->rep = " ";
                list = alist_append(list, t, strg_line);
            }
            t = u;
            continue;
        } else {
            list = alist_append(list, t, strg_line);
            if (n > 0 && t->id == LEX_ID) {
                struct pelist *p = pelookup(pe, t->rep);
                if (p)
                    p->expand++;
            } else if (t->id == LEX_SHARP || t->id == LEX_DSHARP) {
                lex_t *ts = t;
                lex_pos_t tspos = *ppos;
                t = next();
                if (t->id == LEX_SPACE) {
                    lex_t *u = skip(NULL, next);
                    if (u->id != LEX_NEWLINE && u->id != LEX_EOI) {
                        t->rep = " ";
                        list = alist_append(list, t, strg_line);
                    }
                    t = u;
                }
                if (ts->id == LEX_DSHARP) {
                    if (list->next->data == ts || (t->id == LEX_NEWLINE || t->id == LEX_EOI)) {
                        err_issuep(&tspos, ERR_PP_DSHARPPOS);
                        return t;
                    } else if (t->id == LEX_DSHARP) {
                        err_issuep(ppos, ERR_PP_TWODSHARP);
                        return t;
                    }
                    ts->id = LEX_PASTEOP;
                    sharp = 1;
                } else if (n >= 0) {
                    if (t->id != LEX_ID || !pelookup(pe, t->rep)) {
                        err_issuep(&tspos, ERR_PP_NEEDPARAM);
                        return t;
                    }
                    ts->id = LEX_STROP;
                    sharp = 1;
                }
                continue;
            }
        }
        t = next();
    }

    {    /* installation */
        struct mtab *p;

        if (ISPREDMCR(name)) {
            p = lookup(name);
            if (p && p->f.predef) {
                err_issuep(&idpos, ERR_PP_PMCRREDEF, name);
                return t;
            }
        } else if (strcmp(name, "defined") == 0) {
            err_issuep(&idpos, ERR_PP_MCRDEF);
            return t;
        }
        p = add(name, &idpos, alist_toarray(list, *strg_tok), param);
        if (p) {
            if (nppname++ == TL_PPNAME_STD) {
                err_issuep(&idpos, ERR_PP_MANYPPID);
                err_issuep(&idpos, ERR_PP_MANYPPIDSTD, (long)TL_PPNAME_STD);
            }
            if (n >= 0) {
                p->func.argno = n;
                p->func.pe = pe;
                if (sharp && n > 0)
                    chkexp(pe, p->rlist);
            }
            if (sharp)
                p->f.sharp = 1;
            if ((p = conflict(p->name)) != NULL) {
                err_issuep(&idpos, ERR_PP_LONGID, p->name, &p->pos);
                err_issuep(&idpos, ERR_PP_LONGIDSTD, (long)TL_INAME_STD);
            }
        }
    }

    return t;
}


/*
 *  concatenates strings
 */
const char *concat(const char *s1, const char *s2)
{
    size_t sn;
    char *pbuf;

    assert(s1);
    assert(s2);

    sn = strlen(s1);
    pbuf = snbuf(sn+strlen(s2) + 1, 0);
    strcpy(pbuf, s1);
    strcat(pbuf+sn, s2);

    return pbuf;
}


/*
 *  permutates tokens to check validity of successive ##'s
 */
static const char *perm(const char *arr[][2], int l, int u)
{
    static const char *ta[MAXDS][2];

    int i;
    const char *t, *buf;

    assert(arr);
    assert(u <= MAXDS);
    assert(mcr_mpos);

    if (l == u) {
        memcpy(ta, arr, sizeof(ta));
        for (i = 0; i < u; ) {
            alist_t *glist;
            const char **pp, *p = ta[i][0],*n = ta[i][1];
            buf = concat(p, n);
            glist = lex_run(buf, mcr_mpos);
            if (!glist)
                buf = "";
            else if (glist->next != glist) {
                diagds = 0;
                return buf;
            } else
                buf = T(glist->data)->rep;
            for (pp = (const char **)&ta[++i]; pp < (const char **)&ta[u]; pp++)
                if (*pp == p || *pp == n)
                    *pp = buf;
        }
        return NULL;
    }

    for (i = l; i < u; i++) {
        if (i != l)
            swap(l, i);
        if ((buf = perm(arr, l+1, u)) != NULL)
            break;
        if (i != l)
            swap(l, i);
    }

    return buf;
}


/*
 *  checks if the evaluation order of ## affects program validity
 */
static const char *deporder(alist_t *list)
{
    static const char *arr[MAXDS][2];

    int i, n;
    alist_t *glist;
    const char *buf;

    assert(mcr_mpos);

    if (!list || (n = alist_length(list)) <= 2)
        return NULL;

    if (n > MAXDS+1) {
        for (i=0, list=list->next; i < n-1; i++, list=list->next) {
            buf = concat(list->data, list->next->data);
            glist = lex_run(buf, mcr_mpos);
            if (glist && glist->next != glist)
                return buf;
        }
        return NULL;
    }

    for (i=0, list=list->next; i < n-1; i++, list=list->next) {
        arr[i][0] = list->data;
        arr[i][1] = list->next->data;
    }

    return perm(arr, 0, n-1);
}


/*
 *  concatenates two tokens
 */
static lex_t *paste(lex_t *t1, lex_t *t2, struct plist *pl, lxl_t *list, alist_t **pdsl)
{
    static lex_t empty = {
        LEX_SPACE,
        0,
        ""
    };

    struct plist *p;
    node_t *q = NULL;    /* lex_t */
    const char *buf;
    alist_t *glist, *r;

    assert(t1);
    assert(t2);
    assert(pdsl);

    if (pl) {
        if (t1->id == LEX_ID && !t1->blue && (p = plookup(pl, t1->rep)) != NULL) {
            if (*(q = p->rlist) != NULL) {
                for (; q[1]; q++)
                    lxl_append(list, LXL_KTOK, *q);
                t1 = *q;
            } else
                t1 = &empty;
        }
        q = NULL;
        if (t2->id == LEX_ID && (p = plookup(pl, t2->rep)) != NULL) {
            if ((t2 = p->rlist[0]) != NULL) {
                q = p->rlist + 1;
            } else
                t2 = &empty;
        }
    }
    assert(t1);
    assert(t2);

    if (diagds) {
        if (!*pdsl)
            *pdsl = alist_append(*pdsl, (void *)t1->rep, strg_line);
        *pdsl = alist_append(*pdsl, (void *)t2->rep, strg_line);
    }

    assert(mcr_mpos);
    buf = concat(t1->rep, t2->rep);
    glist = lex_run(buf, mcr_mpos);
    if (!glist) {
        err_issuep(mcr_mpos, ERR_PP_EMPTYTOKMADE);
        return &empty;
    } else if (glist->next != glist) {
        err_issuep(mcr_mpos, ERR_PP_INVTOKMADE, buf);
        diagds = 0;
    }
    if (diagds && q && *q) {
        if ((buf = deporder(*pdsl)) != NULL) {
            err_issuep(mcr_mpos, ERR_PP_ORDERDS);
            err_issuep(mcr_mpos, ERR_PP_ORDERDSEX, buf);
            diagds = 0;
         }
         *pdsl = NULL;
    }
    for (r = glist->next; r != glist; r = r->next)
        lxl_append(list, LXL_KTOK, r->data);
    if (q && *q) {
        lxl_append(list, LXL_KTOK, r->data);
        while (q[1])
            lxl_append(list, LXL_KTOK, *q++);
        t1 = *q;
    } else
        t1 = r->data;

    ((lex_t *)t1)->blue = 1;    /* should not be recognized as argument */
    return t1;
}


/*
 *  looks for the next non-space token in a token array
 */
static node_t *nextnsp(node_t pp[])    /* lex_t */
{
    assert(pp);

    if (*pp)
        while (T(*pp)->id == LEX_SPACE)
            pp++;

    return pp;
}


/*
 *  stringifies a macro parameter
 */
static lex_t *stringify(node_t **pq, struct plist *pl)    /* lex_t */
{
    int size = 20;
    struct plist *p;
    char *buf, *pb;
    node_t *r;    /* lex_t */
    lex_t *t;

    assert(pq);
    assert(*pq);
    assert(**pq);
    assert(pl);

    *pq = nextnsp(*pq);
    assert(T(**pq)->id == LEX_ID);
    p = plookup(pl, T(**pq)->rep);
    pb = buf = ARENA_ALLOC(strg_line, sizeof(*buf) * size);
    *pb++ = '"';
    if (p)
        for (r = p->rlist; *r; r++) {
            const char *s = T(*r)->rep;
            while (*s) {
                if (T(*r)->id == LEX_SCON && (*s == '"' || *s == '\\'))
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

    assert(mcr_mpos);
    for (pb = buf+1; *pb && *pb != '"'; pb++)
        if (*pb == '\\')
            pb++;    /* cannot be NUL */
    if (!(pb[0] == '"' && pb[1] == '\0'))
        err_issuep(mcr_mpos, ERR_PP_INVSTRMADE, buf);

    t = ARENA_ALLOC(strg_line, sizeof(*t));
    t->id = LEX_SCON;
    t->rep = buf;
    t->blue = 0;

    return t;
}


/*
 *  handles # and ## operators
 */
int sharp(node_t **pq, lex_t *t1, struct plist *pl, lxl_t *list)    /* lex_t */
{
    lex_t *t2;
    alist_t *dsl = NULL;
    const char *buf;
    int nend = 0, meet = 0;

    assert(pq);
    assert(t1);
    assert(mcr_mpos);

    diagds = (main_opt()->addwarn || main_opt()->std);

    while (1) {
        if (t1->id == LEX_STROP && pl) {
            assert(!nend);
            assert(!meet);
            lxl_append(list, LXL_KSTART, NULL, mcr_mpos);
            meet = nend = 1;
            t1 = stringify(pq, pl);
            continue;
        } else if (t1->id != LEX_SPACE || isempty(t1)) {
            node_t *r = nextnsp(*pq);    /* lex_t */
            if (*r && T(*r)->id == LEX_PASTEOP) {
                *pq = nextnsp(r+1) + 1;
                t2 = (*pq)[-1];
                if (!nend) {
                    lxl_append(list, LXL_KSTART, NULL, mcr_mpos);
                    nend = 1;
                }
                if (t2->id == LEX_STROP && pl) {
                    t2 = stringify(pq, pl);
                    meet |= 0x01;
                }
                t1 = paste(t1, t2, pl, list, &dsl);
                meet |= 0x02;
                continue;
            }
        }
        break;
    }
    if (diagds && dsl && (buf = deporder(dsl)) != NULL) {
        err_issuep(mcr_mpos, ERR_PP_ORDERDS);
        err_issuep(mcr_mpos, ERR_PP_ORDERDSEX, buf);
    }
    if (nend) {
        if (t1->rep[0] != '\0')
            lxl_append(list, LXL_KTOK, t1);
        lxl_append(list, LXL_KEND, NULL, mcr_mpos);
    }

    if (meet == 0x03)
        err_issuep(mcr_mpos, ERR_PP_ORDERSDS);

    return nend;
}


/*
 *  expands macros from arguments
 */
static lxl_t *exparg(const alist_t *list)
{
    lex_t *t;
    ctx_t *ctx;
    lxl_t *elist = lxl_tolxl(list);

    lxl_append(elist, LXL_KEOL);
    ctx = ctx_push(CTX_TNORM);
    ctx->list = elist;
    ctx->cur = elist->head;

    while ((t = lxl_next())->id != LEX_EOI) {
        assert(t->id != LEX_NEWLINE);
        if (t->id == LEX_ID) {
            const lex_pos_t *ppos = mcr_mpos;
            assert(apos);
            if (!ppos)
                mcr_mpos = apos;
            mcr_expand(t, NULL);
            mcr_mpos = ppos;
        }
    }

    assert(ctx_cur->cur->kind == LXL_KEOL);
    ctx_cur->cur->kind = LXL_KEND;    /* overrides LXL_KEOL */
    ctx_cur->cur->u.e.n = NULL;
    ctx_cur->cur->u.e.ppos = PPOS(apos);
    ctx_pop();

    return elist;
}


#define ISNL(nl) (t->id == LEX_NEWLINE && ((nl)=t, !lex_direc))

/*
 *  skips spaces and newlines in macro arguments
 */
static lex_t *skipspnl(lex_t **pnl)
{
    lex_t *t;

    assert(pnl);

    while ((t = lxl_next())->id == LEX_SPACE)
        continue;
    if (ISNL(*pnl)) {
        while ((t = lxl_next())->id == LEX_SPACE || ISNL(*pnl))
            continue;
        if (t->id == LEX_SHARP)
            err_issuep(&lex_cpos, ERR_PP_DIRECINARG);
    }

    return t;
}


/*
 *  recognizes arguments for function-like macros
 */
static struct plist *recarg(struct mtab *p)
{
    int level = 1;
    int errarg = 0;
    lex_t *t, *nl = NULL;
    unsigned long n = 0;
    alist_t *tl = NULL;
    struct plist *pl = NULL;
    lex_pos_t prnpos;

    assert(p);
    assert(ctx_cur->cur->u.t.tok->id == LEX_ID);
    assert(apos);

    ctx_cur->cur->kind = LXL_KTOKI;
    while ((t = lxl_next())->id != '(')
        continue;
    prnpos = lex_cpos;
    t = skipspnl(&nl);

    while (t->id != LEX_EOI && (t->id != LEX_NEWLINE || (nl=t, !lex_direc))) {
        if (t->id == LEX_SPACE || ISNL(nl)) {
            lex_t *u;
            u = skipspnl(&nl);
            if (!(level == 1 && (u->id == ',' || u->id == ')')) && u->id != LEX_EOI &&
                (!lex_direc || u->id != LEX_NEWLINE)) {
                t->id = LEX_SPACE;
                t->rep = " ";
                tl = alist_append(tl, t, strg_line);
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
                    tl = alist_append(tl, t, strg_line);
                } else {
                    if (t->id == ',' || p->func.argno > 0) {
                        if (!tl) {
                            if (n++ == p->func.argno) {
                                err_issuep(PPOS(&lex_cpos), ERR_PP_MANYARG1, p->name);
                                errarg = 1;
                            } else if (n <= p->func.argno)
                                err_issuep(PPOS(&lex_cpos), ERR_PP_EMPTYARG, p->name);
                            if (n == TL_ARGP_STD+1 && !errarg) {
                                err_issuep(PPOS(&lex_cpos), ERR_PP_MANYARG2, p->name);
                                err_issuep(PPOS(&lex_cpos), ERR_PP_MANYARGSTD, (long)TL_ARGP_STD);
                            }
                        }
                        if (n <= p->func.argno) {
                            struct pelist *pe;
                            assert(n > 0);
                            pe = pelookup(p->func.pe, T(p->func.param[n-1])->rep);
                            assert(pe);
                            pl = padd(pl, T(p->func.param[n-1])->rep, tl,
                                      (pe->expand)? exparg(tl): NULL);
                        }
                        tl = NULL;
                    }
                    if (t->id == ')')
                        goto ret;
                    t = skipspnl(&nl);
                    continue;
                }
                break;
            case '(':
                level++;
            default:
                if (!tl) {
                    if (n++ == p->func.argno) {
                        err_issuep(PPOS(&lex_cpos), ERR_PP_MANYARG1, p->name);
                        errarg = 1;
                    }
                    if (n == TL_ARGP_STD+1 && !errarg) {
                        err_issuep(PPOS(&lex_cpos), ERR_PP_MANYARG2, p->name);
                        err_issuep(PPOS(&lex_cpos), ERR_PP_MANYARGSTD, (long)TL_ARGP_STD);
                    }
                }
                tl = alist_append(tl, t, strg_line);
                break;
        }
        t = lxl_next();
    }

    ret:
        if (level > 0)
            err_issuep(PPOS(&prnpos), ERR_PP_UNTERMARG, p->name);
        else if (n < p->func.argno) {
            err_issuep(PPOS(&lex_cpos), ERR_PP_INSUFFARG, p->name);
            while (n++ < p->func.argno)
                pl = padd(pl, T(p->func.param[n-1])->rep, tl, lxl_new(strg_line));
        }

        if ((t->id == LEX_EOI || t->id == LEX_NEWLINE) && nl)
            lxl_append(ctx_cur->list, LXL_KTOK, nl);

        return pl;
}

#undef ISNL


/*
 *  paints tokens being expanded in an expanded replacement list
 */
static void paint(lxl_t *list)
{
    lxl_node_t *p;
    struct emlist *pe;

    assert(list);
    assert(apos);

    for (p = list->head; p; p = p->next) {
        switch(p->kind) {
            case LXL_KTOKI:
                continue;
            case LXL_KTOK:
                if (p->u.t.tok->id == LEX_ID) {
                    pe = elookup(p->u.t.tok->rep);
                    if (EXPANDING(pe)) {
                        p->u.t.blue = 1;
                        if (pe->metend) {
                            err_issuep(PPOS(apos), ERR_PP_UNSPCRECUR, pe->name);
                            pe->metend = 0;    /* issues once per name */
                        }
                    }
                }
                break;
            case LXL_KSTART:
            case LXL_KEND:
                /* since painting starts after expansion of arguments,
                   necessary to stop making expanding context */
                p->u.e.n = NULL;
                break;
            default:
                break;
        }
    }
}


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
 *  expands an identifier if it denotes a macro;
 *  in fact, the parameter t is not necessary but taken for assertion
 */
int (mcr_expand)(lex_t *t, const lex_pos_t *ppos)
{
    node_t *q;    /* lex_t */
    struct mtab *p;
    struct emlist *pe;
    struct plist *pl = NULL;

    assert(t);
    assert(ctx_cur->cur->kind == LXL_KTOK);
    assert(ctx_cur->cur->u.t.tok == t);

    p = lookup(t->rep);
    if (!p || ctx_cur->cur->u.t.blue)
        return 0;

    if (ISPREDMCR(t->rep) && snlen(t->rep, 9) < 9) {
        if (strcmp(t->rep, "__FILE__") == 0) {
            assert(!p->rlist[1]);
            T(p->rlist[0])->rep = mkstr((in_cpos.mf)? in_cpos.mf: in_cpos.g.f, strg_line);
        } else if (strcmp(t->rep, "__LINE__") == 0) {
            char *buf;
            assert(!p->rlist[1]);
            buf = ARENA_ALLOC(strg_line, BUFN + 1);
            sprintf(buf, "%lu", (in_cpos.my > 0)? in_cpos.my: in_cpos.g.y);
            T(p->rlist[0])->rep = buf;
        }
    }

    assert(!mcr_mpos || apos);
    if (!mcr_mpos) {
        assert(ppos);
        apos = ARENA_ALLOC(strg_line, sizeof(*apos));
        *(lex_pos_t *)apos = *ppos;
    }
    ppos = mcr_mpos;

    if (p->f.flike) {
        ctx_push(CTX_TPEEK);
        while ((t = lxl_next())->id == LEX_SPACE || (t->id == LEX_NEWLINE && !lex_direc))
            continue;
        ctx_pop();
        if (t->id == '(') {
            ctx_push(CTX_TIGNORE);
            pl = recarg(p);
            ctx_pop();
        }
    }

    if (!ppos)
        mcr_mpos = apos;

    if (!p->f.flike || t->id == '(') {
        lxl_t *list;
        struct plist *r;
        list = lxl_new(strg_line);
        mcr_eadd(p->name);
        lxl_append(list, LXL_KSTART, p->name, mcr_mpos);
        if (pl)
            for (q = p->func.param; *q; q++) {
                r = plookup(pl, T(*q)->rep);
                if (r && r->elist)
                    paint(r->elist);
            }
        for (q = p->rlist; *q; ) {
            t = *q++;
            if (p->f.sharp && sharp(&q, t, pl, list))
                continue;
            else if (t->id == LEX_ID) {
                if (pl && (r = plookup(pl, t->rep)) != NULL) {
                    lxl_append(list, LXL_KSTART, NULL, mcr_mpos);
                    lxl_insert(list, list->tail, lxl_copy(r->elist));
                    lxl_append(list, LXL_KEND, NULL, mcr_mpos);
                } else {
                    lxl_append(list, LXL_KTOK, t);
                    pe = elookup(t->rep);
                    if (EXPANDING(pe)) {
                        list->tail->u.t.blue = 1;
                        if (pe->metend) {
                            err_issuep(PPOS(apos), ERR_PP_UNSPCRECUR, pe->name);
                            pe->metend = 0;    /* issues once per name */
                        }
                    }
                }
            } else
                lxl_append(list, LXL_KTOK, t);
        }
        lxl_append(list, LXL_KEND, p->name, ppos);
        mcr_edel(p->name);
        ctx_cur->cur->kind = LXL_KTOKI;
        lxl_insert(ctx_cur->list, ctx_cur->cur, list);
    } else {
        mcr_mpos = ppos;
        return 0;
    }

    mcr_mpos = ppos;
    return 1;
}


/*
 *  (predefined macros) adds a macro into the macro table
 */
static void addpr(const char *name, int tid, const char *val)
{
    static lex_pos_t pos = { { -1, 1, "<built-in>", 1 }, 1 };

    lex_t *t;
    struct mtab *p;
    alist_t *list = NULL;

    assert(name);
    assert(tid == LEX_SCON || tid == LEX_PPNUM);
    assert(val);
    assert(tid != LEX_PPNUM || isdigit(*(unsigned char *)val));
    assert(!mcr_redef(name));
    assert(ISPREDMCR(name));

    if (*val) {
        t = ARENA_ALLOC(strg_perm, sizeof(*t));
        t->id = tid;
        if (tid == LEX_SCON && *val != '"')
            val = mkstr(val, strg_perm);
        t->rep = val;
        t->blue = 0;
        list = alist_append(NULL, t, strg_line);
    }

    p = add(name, &pos, alist_toarray(list, strg_perm), NULL);
    assert(p);
    p->f.predef = 1;
}


/*
 *  (command line) adds or removes macro definitions to parse later
 */
void (mcr_cmd)(int del, const char *arg)
{
    struct cmdlist *p;

    assert(arg);

    MEM_NEW(p);
    p->del = del;
    p->arg = arg;
    cmdlist = list_push(cmdlist, p);
}


/*
 *  retrieves a token from a token list
 */
static lex_t *nextcl(void)
{
    static lex_t eoi = {
        LEX_EOI,
        0,
        ""
    };

    assert(cltok);

    return (*cltok)? *cltok++: &eoi;
}


/*
 *  (predefined, command-line) initializes macros;
 *  ASSUMPTION: hosted implementation assumed
 */
void (mcr_init)(void)
{
    static char pdate[] = "\"May  4 1979\"",
                ptime[] = "\"07:10:05\"";
    static lex_pos_t pos = { { -1, 1, "<command-line>", 1 }, 1 };

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

    cmdlist = list_reverse(cmdlist);
    while (cmdlist) {
        void *c;
        lex_t *t;
        const char *name;

        cmdlist = list_pop(cmdlist, &c);
        strg_tok = &strg_perm;    /* tokens go to strg_perm */
        cltok = alist_toarray(lex_run(((struct cmdlist *)c)->arg, &pos), strg_line);
        strg_tok = &strg_line;
        if (!((struct cmdlist *)c)->del)
            mcr_define(1, nextcl, &pos);
        else {
            t = skip(NULL, nextcl);
            if (t->id != LEX_ID)
                err_issuep(&pos, ERR_PP_NOMCRID);
            else {
                name = t->rep;
                mcr_del(name, &pos);
                if (skip(NULL, nextcl)->id != LEX_EOI)
                    err_issuep(&pos, ERR_PP_EXTRATOKENCL, name);
            }
        }
        MEM_FREE(c);
    }
}


/*
 *  frees storages for handling macros
 */
void mcr_free(void)
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
 *  prints the macro table for debugging
 */
static void print(FILE *fp)
{
    int i;
    struct mtab *p;
    node_t *q;    /* lex_t */

    assert(fp);

    for (i = 0; i < NELEM(mtab.t); i++)
        for (p = mtab.t[i]; p; p = p->link) {
            if (!p->name)
                continue;
            fputs(p->name, fp);
            if (p->f.flike) {
                putc('(', fp);
                for (q = p->func.param; *q; q++) {
                    fputs(T(*q)->rep, fp);
                    if (q[1])
                        fputs(", ", fp);
                }
                putc(')', fp);
            }
            fputs(" = ", fp);
            for (q = p->rlist; *q; q++)
                fputs(T(*q)->rep, fp);
            fprintf(fp, " @%s\n", lex_outpos(&p->pos));
        }
}


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
            fprintf(fp, "%s(%d)%s ", p->name, p->count, (p->metend)? "!": "");
    fputs("]\n", fp);
}
#endif    /* !NDEBUG */

/* end of mcr.c */
