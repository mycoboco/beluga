/*
 *  macro for preprocessing
 */

#include <ctype.h>         /* isdigit */
#include <stddef.h>        /* size_t, NULL */
#include <stdio.h>         /* FILE, fprintf, sprintf, putc, fputs */
#include <string.h>        /* memcpy, strcmp, strcpy, strcat, strlen, strncpy */
#include <time.h>          /* time_t, time, ctime */
#include <cbl/arena.h>     /* arena_t, ARENA_ALLOC, ARENA_CALLOC */
#include <cbl/assert.h>    /* assert */
#include <cbl/memory.h>    /* MEM_NEW, MEM_FREE */
#include <cdsl/hash.h>     /* hash_string, hash_new */
#include <cdsl/list.h>     /* list_t, list_push, list_reverse, list_pop */

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

#define MAXDS  6    /* max number of successive #'s to fully diagnose */
#define PETABN 4    /* (parameter expansion table) number of buckets */

/* selectively provides locus for diagnostics */
#define PPOS(p) ((mcr_mpos)? mcr_mpos: p)

#define EXPANDING(p) ((p) && (p)->count > 0)    /* checks if a macro is being expanded */
#define isempty(t)   (*(t)->rep == '\0')        /* true if a space token denotes an empty token */
#define T(p)         ((lex_t *)p)               /* shorthand for cast to lex_t * */

/* helper macro for perm() */
#define swap(i, j)  (t=arr[i][0], arr[i][0]=arr[j][0], arr[j][0]=t,    \
                     t=arr[i][1], arr[i][1]=arr[j][1], arr[j][1]=t)

/* generates a hash key from a pointer */
#define hashkey(p, n) (((unsigned)(p) >> 3) & ((n)-1))

/* (predefined macros) checks if predefined macros */
#define ISPREDMCR(n) ((n)[0] == '_' && (n)[1] == '_')


/* (command list) macros from command line */
struct cmdlist {
    int del;            /* 0: to add, 1: to remove */
    const char *arg;    /* argument string */
};

/* parameter expansion table */
struct petab {
    const char *name;      /* parameter name */
    int expand;            /* # of occurrences for expansion */
    struct petab *link;    /* hash chain */
};

typedef void *node_t;    /* refers to void * for readability */


const lex_pos_t *mcr_mpos;    /* locus for diagnostics */


/* macro table */
static struct mtab {
    const char *name;     /* macro name */
    lex_pos_t pos;        /* definition locus */
    node_t *rlist;        /* (lex_t) replacement list */
    lxl_t *elist;         /* (parameter table) expanded replacement list */
    struct {
        unsigned flike:  1;    /* function-like */
        unsigned sharp:  1;    /* has # or ## */
        unsigned predef: 1;    /* predefined macros */
    } f;
    struct {
        int argno;               /* # of arguments */
        node_t *param;           /* (lex_t) parameters */
        struct petab **petab;    /* parameter expansion table */
    } func;               /* function-like macro */
    struct mtab *link;    /* hash chain */
} *mtab[64];

/* expanding macro table */
static struct etab {
    const char *name;      /* macro name */
    int count;             /* nesting count */
    unsigned metend: 1;    /* true when LXL_KEND encountered */
    struct etab *link;     /* hash chain */
} *etab[64];

static const lex_pos_t *apos;    /* locus for diagnostics */
static int diagds;               /* true if issueing ERR_PP_ORDERDS is enabled */
static list_t *cmdlist;          /* (command list) macros from command line */
static node_t *cltok;            /* (lex_t) list from which nextcl() brings tokens */
static int nppname;              /* number of macros defined */


/*
 *  (parameter expansion table) adds a parameter
 */
static struct petab **peadd(struct petab *tab[], const char *name, int *found)
{
    unsigned h;
    struct petab *p;

    assert(found);

    if (!tab)
        tab = ARENA_CALLOC(*strg_tok, PETABN, sizeof(**tab));

    name = hash_string(name);
    h = hashkey(name, PETABN);
    for (p = tab[h]; p; p = p->link)
        if (p->name == name) {
            *found = 1;
            return tab;
        }

    p = ARENA_ALLOC(*strg_tok, sizeof(*p));
    p->name = name;
    p->expand = 0;
    p->link = tab[h];
    tab[h] = p;
    *found = 0;

    return tab;
}


/*
 *  (parameter expansion table) looks up a parameter
 */
static struct petab *pelookup(struct petab *tab[], const char *name)
{
    unsigned h;
    struct petab *p;

    name = hash_string(name);
    h = hashkey(name, PETABN);
    for (p = tab[h]; p; p = p->link)
        if (p->name == name)
            return p;

    return NULL;
}


/*
 *  (expanding macro table) adds an identifier
 */
void (mcr_eadd)(const char *name)
{
    unsigned h;
    struct etab *p;

    name = hash_string(name);
    h = hashkey(name, NELEM(etab));
    for (p = etab[h]; p; p = p->link)
        if (p->name == name)
            break;

    if (!p) {
        p = ARENA_ALLOC(strg_perm, sizeof(*p));
        p->name = name;
        p->count = 0;
        p->link = etab[h];
        etab[h] = p;
    }
    p->count++;
    p->metend = 0;
}


/*
 *  (expanding macro table) looks up an identifier
 */
static struct etab *elookup(const char *name)
{
    unsigned h;
    struct etab *p;

    name = hash_string(name);
    h = hashkey(name, NELEM(etab));
    for (p = etab[h]; p; p = p->link)
        if (p->name == name)
            return p;

    return NULL;
}


/*
 *  (expanding macro table) removes an identifier
 */
void (mcr_edel)(const char *name)
{
    struct etab *p = elookup(name);

    assert(p);
    if (--p->count == 0)
        p->metend = 0;
    assert(p->count >= 0);
}


/*
 *  (expanding macro table) sets a flag when tokens are taken across LXL_KEND
 */
void (mcr_emeet)(const char *name)
{
    struct etab *p = elookup(name);

    assert(p);
    if (p->count > 0)
        p->metend = 1;
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
 *  (macro table, parameter table) adds an identifier
 */
static struct mtab *add(struct mtab *tab[], const char *name, const lex_pos_t *ppos,
                        node_t list[], node_t param[])     /* lex_t */
{
    unsigned h;
    struct mtab *p;

    assert(tab);
    assert(ppos);

    name = hash_string(name);
    h = hashkey(name, NELEM(mtab));
    for (p = tab[h]; p; p = p->link)
        if (p->name == name) {
            if ((p->f.flike ^ !!param) || !eqtlist(p->rlist, list) ||
                (param && !eqtlist(p->func.param, param)))
                err_issuep(ppos, ERR_PP_MCRREDEF, name, lex_outpos(&p->pos));
            return NULL;
        }

    p = ARENA_CALLOC((tab == mtab)? strg_perm: *strg_tok, 1, sizeof(*p));
    p->name = name;
    p->pos = *ppos;
    p->rlist = list;
    p->f.flike = !!param;
    p->func.argno = -1;
    p->func.param = param;
    p->link = tab[h];
    tab[h] = p;

    return p;
}


/*
 *  (parameter table) adds a macro parameter
 */
static struct mtab **padd(struct mtab *ptab[], const char *name, const lex_pos_t *ppos,
                          const alist_t *rlist, lxl_t *elist)
{
    struct mtab *p;

    if (!ptab)
        ptab = ARENA_CALLOC(strg_line, NELEM(mtab), sizeof(**ptab));

    p = add(ptab, name, ppos, alist_toarray(rlist, strg_line), NULL);
    assert(p);
    p->elist = elist;

    return ptab;
}


/*
 *  (macro table) looks up an identifier
 */
static struct mtab *lookup(struct mtab *tab[], const char *name)
{
    unsigned h;
    struct mtab *p;

    assert(tab);

    name = hash_string(name);
    h = hashkey(name, NELEM(mtab));
    for (p = tab[h]; p; p = p->link)
        if (p->name == name)
            return p;

    return NULL;
}


/*
 *  (macro table) checks if an identifier has been #defined
 */
int (mcr_redef)(const char *name)
{
    return !!lookup(mtab, name);
}


/*
 *  (macro table) #undefines a macro
 */
void (mcr_del)(const char *name, const lex_pos_t *ppos)
{
    struct mtab *p = lookup(mtab, name);

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
static void chkexp(struct petab *tab[], node_t p[])
{
    int first = 0;
    struct petab *q;
    lex_t *t, *pt = NULL;

    assert(tab);
    assert(p);

    while (*p) {
        t = *p;
        switch(t->id) {
            case LEX_ID:
                if (pt) {
                    if (pt->id == LEX_STROP) {
                        pelookup(tab, t->rep)->expand--;
                        pt = NULL;
                        break;
                    } else if (pt->id == LEX_PASTEOP) {
                        if ((q=pelookup(tab, t->rep)) != NULL)
                            q->expand--;
                    } else
                        first = 0;
                }
                pt = t;
                break;
            case LEX_PASTEOP:
                if (!first) {
                    first = 1;
                    if (pt && pt->id == LEX_ID && (q=pelookup(tab, pt->rep)) != NULL)
                        q->expand--;
                }
                pt = t;
                break;
            default:
                first = 0;
                pt = t;
                break;
        }
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
                r = lookup(mtab, p->name);
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
    struct petab **petab = NULL;

    assert(next);
    assert(ppos);

    t = skip(next(), next);
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
        t = skip(next(), next);
        while (t->id == LEX_ID) {
            if (n++ == TL_PARAMP_STD) {
                err_issuep(ppos, ERR_PP_MANYPARAM);
                err_issuep(ppos, ERR_PP_MANYPSTD, (int)TL_PARAMP_STD);
            }
            petab = peadd(petab, t->rep, &dup);
            if (dup) {
                err_issuep(ppos, ERR_PP_DUPNAME, t->rep);
                return t;
            }
            plist = alist_append(plist, t, strg_line);
            t = skip(next(), next);
            if (t->id != ',')
                break;
            t = skip(next(), next);
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
            t = skip(next(), next);
        else if (t->id == LEX_EOI)
            t = &one;
        else
            err_issuep(ppos, ERR_PP_NOEQCL, (n >= 0)? "function": "object", name);
    }
    while (t->id != LEX_NEWLINE && t->id != LEX_EOI) {
        if (t->id == LEX_SPACE) {
            lex_t *u = skip(next(), next);
            if (u->id != LEX_NEWLINE && u->id != LEX_EOI) {
                t->rep = " ";
                list = alist_append(list, t, strg_line);
            }
            t = u;
            continue;
        } else {
            list = alist_append(list, t, strg_line);
            if (n > 0 && t->id == LEX_ID) {
                struct petab *p = pelookup(petab, t->rep);
                if (p)
                    p->expand++;
            } else if (t->id == LEX_SHARP || t->id == LEX_DSHARP) {
                lex_t *ts = t;
                lex_pos_t tspos = *ppos;
                t = next();
                if (t->id == LEX_SPACE) {
                    lex_t *u = skip(next(), next);
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
                    if (t->id != LEX_ID || !petab || !pelookup(petab, t->rep)) {
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
            p = lookup(mtab, name);
            if (p && p->f.predef) {
                err_issuep(&idpos, ERR_PP_PMCRREDEF, name);
                return t;
            }
        } else if (strcmp(name, "defined") == 0) {
            err_issuep(&idpos, ERR_PP_MCRDEF);
            return t;
        }
        p = add(mtab, name, &idpos, alist_toarray(list, *strg_tok), param);
        if (p) {
            if (nppname++ == TL_PPNAME_STD) {
                err_issuep(&idpos, ERR_PP_MANYPPID);
                err_issuep(&idpos, ERR_PP_MANYPPIDSTD, (int)TL_PPNAME_STD);
            }
            if (n >= 0) {
                p->func.argno = n;
                p->func.petab = petab;
                if (sharp && n > 0)
                    chkexp(petab, p->rlist);
            }
            if (sharp)
                p->f.sharp = 1;
            if ((p=conflict(p->name)) != NULL) {
                err_issuep(&idpos, ERR_PP_LONGID, p->name, lex_outpos(&p->pos));
                err_issuep(&idpos, ERR_PP_LONGIDSTD, TL_INAME_STD);
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
    char *pbuf;
    size_t sn = strlen(s1);

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
    assert(apos);

    if (l == u) {
        memcpy(ta, arr, sizeof(ta));
        for (i = 0; i < u; ) {
            alist_t *glist;
            const char **pp, *p = ta[i][0],*n = ta[i][1];
            buf = concat(p, n);
            glist = lex_run(buf, PPOS(apos));
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
        if ((buf=perm(arr, l+1, u)) != NULL)
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

    assert(apos);

    if (!list || (n=alist_length(list)) < 3)
        return NULL;

    if (n > MAXDS+1) {
        for (i=0, list=list->next; i < n-1; i++, list=list->next) {
            buf = concat(list->data, list->next->data);
            glist = lex_run(buf, PPOS(apos));
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
static lex_t *paste(lex_t *t1, lex_t *t2, struct mtab *ptab[], lxl_t *list, alist_t **pdsl)
{
    static lex_t empty = {
        LEX_SPACE,
        0,
        ""
    };

    struct mtab *p;
    node_t *q = NULL;    /* lex_t */
    const char *buf;
    alist_t *glist, *r;

    assert(t1);
    assert(t2);
    assert(pdsl);

    if (ptab) {
        if (t1->id == LEX_ID && !t1->blue && (p=lookup(ptab, t1->rep)) != NULL) {
            if (*(q=p->rlist) != NULL) {
                for (; q[1]; q++)
                    lxl_append(list, LXL_KTOK, *q);
                t1 = *q;
            } else
                t1 = &empty;
        }
        q = NULL;
        if (t2->id == LEX_ID && (p=lookup(ptab, t2->rep)) != NULL) {
            if ((t2=p->rlist[0]) != NULL) {
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

    assert(apos);
    buf = concat(t1->rep, t2->rep);
    glist = lex_run(buf, PPOS(apos));
    if (!glist) {
        err_issuep(PPOS(apos), ERR_PP_EMPTYTOKMADE);
        return &empty;
    } else if (glist->next != glist) {
        err_issuep(PPOS(apos), ERR_PP_INVTOKMADE, buf);
        diagds = 0;
    }
    if (diagds && q && *q) {
        if ((buf=deporder(*pdsl)) != NULL) {
            err_issuep(PPOS(apos), ERR_PP_ORDERDS);
            err_issuep(PPOS(apos), ERR_PP_ORDERDSEX, buf);
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
static lex_t *stringify(node_t **pq, struct mtab *ptab[])    /* lex_t */
{
    struct mtab *p;
    int size = 20;
    char *buf, *pb;
    node_t *r;    /* lex_t */
    lex_t *t;

    assert(pq);
    assert(ptab);
    assert(*pq);
    assert(**pq);

    *pq = nextnsp(*pq);
    assert(T(**pq)->id == LEX_ID);
    p = lookup(ptab, T(**pq)->rep);
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

    assert(apos);
    for (pb = buf+1; *pb && *pb != '"'; pb++)
        if (*pb == '\\')
            pb++;    /* cannot be NUL */
    if (!(pb[0] == '"' && pb[1] == '\0'))
        err_issuep(PPOS(apos), ERR_PP_INVSTRMADE, buf);

    t = ARENA_ALLOC(strg_line, sizeof(*t));
    t->id = LEX_SCON;
    t->rep = buf;
    t->blue = 0;

    return t;
}


/*
 *  handles # and ## operators
 */
int sharp(node_t **pq, lex_t *t1, struct mtab *ptab[], lxl_t *list)    /* lex_t */
{
    lex_t *t2;
    alist_t *dsl = NULL;
    const char *buf;
    int nend = 0, meet = 0;

    assert(pq);
    assert(t1);
    assert(apos);

    diagds = (main_opt()->addwarn || main_opt()->std);

    while (1) {
        if (t1->id == LEX_STROP && ptab) {
            assert(!nend);
            assert(!meet);
            lxl_append(list, LXL_KSTART, NULL, PPOS(apos));
            meet = nend = 1;
            t1 = stringify(pq, ptab);
            continue;
        } else if (t1->id != LEX_SPACE || isempty(t1)) {
            node_t *r = nextnsp(*pq);    /* lex_t */
            if (*r && T(*r)->id == LEX_PASTEOP) {
                *pq = nextnsp(r+1) + 1;
                t2 = (*pq)[-1];
                if (!nend) {
                    lxl_append(list, LXL_KSTART, NULL, PPOS(apos));
                    nend = 1;
                }
                if (t2->id == LEX_STROP && ptab) {
                    t2 = stringify(pq, ptab);
                    meet |= 0x01;
                }
                t1 = paste(t1, t2, ptab, list, &dsl);
                meet |= 0x02;
                continue;
            }
        }
        break;
    }
    if (diagds && dsl && (buf=deporder(dsl)) != NULL) {
        err_issuep(PPOS(apos), ERR_PP_ORDERDS);
        err_issuep(PPOS(apos), ERR_PP_ORDERDSEX, buf);
    }
    if (nend) {
        if (t1->rep[0] != '\0')
            lxl_append(list, LXL_KTOK, t1);
        lxl_append(list, LXL_KEND, NULL, PPOS(apos));
    }

    if (meet == 0x03)
        err_issuep(PPOS(apos), ERR_PP_ORDERSDS);

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

    while ((t=lxl_next())->id != LEX_EOI) {
        assert(t->id != LEX_NEWLINE);
        if (t->id == LEX_ID) {
            const lex_pos_t *ppos = mcr_mpos;
            assert(apos);
            if (!ppos) {
                mcr_mpos = apos;
            }
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

    while ((t=lxl_next())->id == LEX_SPACE)
        continue;
    if (ISNL(*pnl)) {
        while ((t=lxl_next())->id == LEX_SPACE || ISNL(*pnl))
            continue;
        if (t->id == LEX_SHARP)
            err_issuep(&lex_cpos, ERR_PP_DIRECINARG);
    }

    return t;
}


/*
 *  recognizes arguments for function-like macros
 */
static struct mtab **recarg(struct mtab *p)
{
    int level = 1;
    int errarg = 0;
    lex_t *t, *nl = NULL;
    unsigned long n = 0;
    alist_t *tl = NULL;
    struct mtab **ptab = NULL;

    assert(p);
    assert(ctx_cur->cur->u.t.tok->id == LEX_ID);
    assert(apos);

    ctx_cur->cur->kind = LXL_KTOKI;
    while ((t=lxl_next())->id != '(')
        continue;
    t = skipspnl(&nl);

    while (t->id != LEX_EOI && (t->id != LEX_NEWLINE || (nl=t, !lex_direc))) {
        if (t->id == LEX_SPACE || ISNL(nl)) {
            lex_t *u;
            u = skipspnl(&nl);
            if (!(level == 1 && (u->id == ',' || u->id == ')')) && u->id != LEX_EOI &&
                (!lex_direc || u->id != LEX_NEWLINE)) {
                t->id = LEX_SPACE;
                t->rep = " ";
                tl = alist_append(tl, t, strg_ctx);
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
                    tl = alist_append(tl, t, strg_ctx);
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
                                err_issuep(PPOS(&lex_cpos), ERR_PP_MANYARGSTD, (int)TL_ARGP_STD);
                            }
                        }
                        if (n <= p->func.argno) {
                            struct petab *pe;
                            assert(n > 0);
                            pe = pelookup(p->func.petab, T(p->func.param[n-1])->rep);
                            assert(pe);
                            ptab = padd(ptab, T(p->func.param[n-1])->rep, PPOS(&lex_cpos), tl,
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
                        err_issuep(PPOS(&lex_cpos), ERR_PP_MANYARGSTD, (int)TL_ARGP_STD);
                    }
                }
                tl = alist_append(tl, t, strg_ctx);
                break;
        }
        t = lxl_next();
    }

    ret:
        if (level > 0)
            err_issuep(PPOS(&lex_cpos), ERR_PP_UNTERMARG, p->name);
        else if (n < p->func.argno) {
            err_issuep(PPOS(&lex_cpos), ERR_PP_INSUFFARG, p->name);
            while (n++ < p->func.argno)
                ptab = padd(ptab, T(p->func.param[n-1])->rep, PPOS(apos), tl, lxl_new(strg_line));
        }

        if ((t->id == LEX_EOI || t->id == LEX_NEWLINE) && nl)
            lxl_append(ctx_cur->list, LXL_KTOK, nl);

        return ptab;
}

#undef ISNL


/*
 *  paints tokens being expanded in an expanded replacement list
 */
static void paint(lxl_t *list)
{
    lxl_node_t *p;
    struct etab *pe;

    assert(list);

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
    assert(a);
    assert(*s != '"');

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
    struct mtab *p, **ptab = NULL;
    struct etab *pe;

    assert(t);
    assert(ctx_cur->cur->kind == LXL_KTOK);
    assert(ctx_cur->cur->u.t.tok == t);

    p = lookup(mtab, t->rep);
    if (!p || ctx_cur->cur->u.t.blue)
        return 0;

    if (ISPREDMCR(t->rep) && snlen(t->rep, 9) < 9) {
        if (strcmp(t->rep, "__FILE__") == 0) {
            assert(!p->rlist[1]);
            T(p->rlist[0])->rep = mkstr((in_cpos.mf)? in_cpos.mf: in_cpos.f, strg_line);
        } else if (strcmp(t->rep, "__LINE__") == 0) {
            char *buf;
            assert(!p->rlist[1]);
            buf = ARENA_ALLOC(strg_line, (sizeof(unsigned long)*CHAR_BIT+2)/3 + 1);
            sprintf(buf, "%lu", (in_cpos.my > 0)? in_cpos.my: in_cpos.y);
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
        while ((t=lxl_next())->id == LEX_SPACE || (t->id == LEX_NEWLINE && !lex_direc))
            continue;
        ctx_pop();
        if (t->id == '(') {
            ctx_push(CTX_TIGNORE);
            ptab = recarg(p);
            ctx_pop();
        }
    }

    if (!ppos)
        mcr_mpos = apos;

    if (!p->f.flike || t->id == '(') {
        lxl_t *list;
        struct mtab *r;
        list = lxl_new(strg_line);
        mcr_eadd(p->name);
        lxl_append(list, LXL_KSTART, p->name, mcr_mpos);
        if (ptab)
            for (q = p->func.param; *q; q++) {
                r = lookup(ptab, T(*q)->rep);
                if (r && r->elist)
                    paint(r->elist);
            }
        for (q = p->rlist; *q; ) {
            t = *q++;
            if (p->f.sharp && sharp(&q, t, ptab, list))
                continue;
            else if (t->id == LEX_ID) {
                if (ptab && (r=lookup(ptab, t->rep)) != NULL)
                    lxl_insert(list, list->tail, lxl_copy(r->elist));
                else {
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
        lxl_append(list, LXL_KEND, p->name, (ppos)? ppos: NULL);
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
    static lex_pos_t pos = { -1, 1, "<built-in>", 1, 1 };

    lex_t *t;
    struct mtab *p;
    alist_t *list = NULL;

    assert(name);
    assert(tid == LEX_SCON || tid == LEX_PPNUM);
    assert(val);
    assert(tid != LEX_PPNUM || isdigit(*val));
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

    p = add(mtab, name, &pos, alist_toarray(list, strg_perm), NULL);
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
    static lex_pos_t pos = { -1, 1, "<command-line>", 1, 1 };

    time_t tm = time(NULL);
    char *p = ctime(&tm);    /* Fri May  4 07:10:05 1979\n */

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
            t = skip(nextcl(), nextcl);
            if (t->id != LEX_ID)
                err_issuep(&pos, ERR_PP_NOMCRID);
            else {
                name = t->rep;
                mcr_del(name, &pos);
                if (skip(nextcl(), nextcl)->id != LEX_EOI)
                    err_issuep(&pos, ERR_PP_EXTRATOKENCL, name);
            }
        }
        MEM_FREE(c);
    }
}


/*
 *  prints the macro table for debugging
 */
static void print(FILE *fp, struct mtab *tab[])
{
    int i;
    struct mtab *p;
    node_t *q;    /* lex_t */

    assert(fp);

    if (!tab)
        tab = mtab;

    for (i = 0; i < NELEM(mtab); i++)
        for (p = tab[i]; p; p = p->link) {
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
            if (p->elist)
                lxl_print(p->elist, NULL, fp);
        }
}


/*
 *  (expanding macro table) prints for debugging
 */
void (mcr_eprint)(FILE *fp)
{
    int i;
    struct etab *p;

    assert(fp);

    fputs("[ ", fp);
    for (i = 0; i < NELEM(etab); i++)
        for (p = etab[i]; p; p = p->link) {
            if (p->count > 0)
                fprintf(fp, "%s(%d)%s ", p->name, p->count, (p->metend)? "!": "");
        }
    fputs("]\n", fp);
}

/* end of mcr.c */
