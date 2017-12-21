/*
 *  include for preprocessing
 */

#include <stddef.h>        /* NULL, size_t */
#include <stdio.h>         /* FILE, fopen, fclose */
#include <string.h>        /* strlen, strcpy, strrchr, strtok */
#include <cbl/assert.h>    /* assert */
#include <cbl/arena.h>     /* ARENA_ALLOC */
#include <cdsl/hash.h>     /* hash_new, hash_string */
#include <cdsl/list.h>     /* list_t, list_push, list_reverse, list_free, LIST_FOREACH */

#include "common.h"
#include "cond.h"
#include "err.h"
#include "in.h"
#include "mg.h"
#include "strg.h"
#include "util.h"
#include "inc.h"

/* stringification */
#define str(x) #x
#define xstr(x) str(x)

/* separators */
#ifndef DIR_SEPARATOR
#define DIR_SEPARATOR /
#endif    /* !DIR_SEPARATOR */

#ifndef PATH_SEPARATOR
#define PATH_SEPARATOR :
#endif    /* !PATH_SEPARATOR */

#define DSEP (xstr(DIR_SEPARATOR)[0])    /* separator for directory */
#define PSEP (xstr(PATH_SEPARATOR))      /* separator for path */

/* system header directories */
#ifndef SYSTEM_HEADER_DIR
#define SYSTEM_HEADER_DIR "/usr/include:/usr/local/include"
#endif    /* !SYSTEM_HEADER_DIR */

/* rpath() or nop depending on HAVE_REALPATH */
#ifdef HAVE_REALPATH
#define RPATHNOP(p) (rpath(p))
#else    /* !HAVE_REALPATH */
#define RPATHNOP(p) (p)
#endif    /* HAVE_REALPATH */


/* marks end of inc_chain */
static inc_t sentinel, *psentinel = &sentinel;


/* #include chain;
   initialized because it may be used before inc_init() */
inc_t **inc_chain = &psentinel;

int inc_level;    /* nesting level of #include's */


static list_t *rpl[4];              /* raw path lists (user, system, built-in, after) */
static int syslev = -1;             /* nesting level of system header */
static inc_t *incinfo[TL_INC+1];    /* #include chain */


/*
 *  adds raw #include paths to parse later
 */
void (inc_add)(const char *s, int n)
{
    char *p;
    list_t *iter;

    assert(s);
    assert(n >= 0 && n < NELEM(rpl));

    p = strcpy(snbuf(strlen(s)+1, 0), s);
    for (; (p = strtok(p, PSEP)) != NULL; p = NULL) {
        s = hash_string(p);
        LIST_FOREACH(iter, rpl[n]) {
            if (RPATHNOP(iter->data) == RPATHNOP(s))
                break;
        }
        if (!iter)
            rpl[n] = list_push(rpl[n], (void *)s);
    }
}


/*
 *  extracts the current working directory from header names
 */
static const char *getcwd(const char *h)
{
    const char *p;

    assert(h);
    assert(DSEP != '\0');

    p = strrchr(h, DSEP);
    if (!p)
        return "";

    return hash_new(h, p - h);
}


/*
 *  prepares to process #include's;
 *  no duplication check
 */
void (inc_init)(void)
{
    int i;
    list_t *p, *q;

    inc_add(SYSTEM_HEADER_DIR, 2);
    for (i = 0; i < NELEM(rpl); i++)
        rpl[i] = list_reverse(rpl[i]);
    LIST_FOREACH(p, rpl[0]) {
        for (i = 1; p->data && i < NELEM(rpl); i++)
            LIST_FOREACH(q, rpl[i]) {
                if (RPATHNOP(p->data) == RPATHNOP(q->data)) {
                    p->data = NULL;
                    break;
                }
            }
    }
    rpl[0] = list_push(rpl[0], "");

    incinfo[NELEM(incinfo)-1] = &sentinel;
    inc_chain = &incinfo[NELEM(incinfo)-1];

#ifndef JSON_DIAG
    if (main_opt()->verbose) {
        fprintf(stderr, "#include search starts here:\n");
        for (i = 0; i < NELEM(rpl); i++) {
            LIST_FOREACH(p, rpl[i]) {
                if (!p->data)
                    continue;
                if (((char *)p->data)[0] == '\0') {
                    if (i == 0)
                        fputs(" . (only for #include \"...\")\n", stderr);
                } else
                    fprintf(stderr, " %s\n", (char *)p->data);
            }
            if (main_opt()->nostdinc && i == 1)
                i++;
        }
    }
#endif    /* !JSON_DIAG */
}


/*
 *  frees storages for handling #include's
 */
void (inc_free)(void)
{
    int i;

    for (i = 0; i < NELEM(rpl); i++)
        list_free(&rpl[i]);
}


/*
 *  builds full paths for header names
 */
static const char *build(const char *p, const char *h, size_t *pn)
{
    char *full;
    size_t np, nh;

    assert(p);
    assert(h);
    assert(*h);
    assert(pn);
    assert(DSEP != '\0');

    np = strlen(p);
    if (np == 1 && p[0] == '.')
        p = "", np = 0;
    nh = strlen(h);
    full = snbuf(np+nh+2, 0);    /* +2 for DSEP and NUL */

    if (h[0] != DSEP && np > 0) {
        strcpy(full, p);
        if (p[np-1] != DSEP) {
            full[np++] = DSEP,
            full[np] = '\0';
        }
    } else
        np = 0;
    strcpy(full+np, h);
    full[np+nh-1] = '\0';    /* -1 to remove " or > */
    *pn = np;

    return full;
}


/*
 *  recognizes header names
 */
int (inc_start)(const char *fn, const lmap_t *hpos)
{
    int i, q;
    FILE *fp;
    list_t *p;
    size_t n;
    const char *ffn = NULL, *c;

    assert(fn);
    assert(*fn == '<' || *fn == '"');
    assert(hpos);

    c = getcwd(lmap_pfrom(hpos->from)->u.i.f);

    q = (*fn++ == '"');
    /* closing character will be deleted later */
    if (fn[1] == '\0') {
        err_dpos(hpos, ERR_PP_EMPTYHDR);
        return 0;
    }

    assert(rpl[0]->data && rpl[2]->data);
    for (i = 0; i < NELEM(rpl); i++) {
        LIST_FOREACH(p, rpl[i]) {
            if (!p->data)
                continue;
            if (((char *)p->data)[0] == '\0') {
                if (!q)
                    continue;
                ffn = build(c, fn, &n);
            } else
                ffn = build(p->data, fn, &n);
            if ((fp = fopen(ffn, "r")) != NULL)
                goto found;
        }
        if (main_opt()->nostdinc && i == 1) {
            if (!ffn)
                ffn = build("", fn, &n);
            i++;
        }
    }
    err_dpos(hpos, ERR_PP_NOINCFILE, ffn + n);
    return 0;

    found:
        if (inc_level == TL_INC_STD)
            (void)(err_dpos(hpos, ERR_PP_MANYINCW) &&
                   err_dpos(hpos, ERR_PP_MANYINCSTD, (long)TL_INC_STD));
        if (inc_level == TL_INC) {
            fclose(fp);
            err_dpos(hpos, ERR_PP_MANYINC);
            return 0;
        }
        c = rpath(ffn);
        if (mg_isguarded(c)) {
            fclose(fp);
            return 0;
        }
        if (i > 0 && syslev < 0)
            syslev = inc_level;
        hpos = lmap_mstrip(hpos);
        lmap_from = lmap_include(c, (main_opt()->path == 0 && strlen(c) <= strlen(ffn))?
                                        c: hash_string((main_opt()->path == 2)? ffn+n: ffn), hpos,
                                 (syslev >= 0));
        lmap_flset(c);
        in_switch(fp, hpos->u.n.py-in_py);

        return 1;
}


/*
 *  pushes the current context into the #include chain
 */
void (inc_push)(FILE *fp, int bs)
{
    inc_t *p;

    assert(inc_chain > &incinfo[0]);

    inc_level++;
    if (!*--inc_chain)
        *inc_chain = ARENA_ALLOC(strg_perm, sizeof(**inc_chain));
    p = *inc_chain;

    p->fptr = fp;
    p->bs = bs;
    p->cond = cond_list;
    p->mgstate = mg_state;
    p->mgname = mg_name;
    cond_list = NULL;
    mg_state = MG_SINCLUDE;
}


/*
 *  pops a context from the #include chain
 */
FILE *(inc_pop)(FILE *fp, sz_t *ppy)
{
    inc_t *p;
    const lmap_t *pos;

    assert(fp);
    assert(inc_level > 0);
    assert(inc_chain < &incinfo[NELEM(incinfo)-1]);

    inc_level--;
    fclose(fp);

    p = *inc_chain++;

    cond_list = p->cond;
    mg_state = p->mgstate;
    mg_name = p->mgname;
    pos = lmap_pfrom(lmap_from)->from;
    assert(pos->type == LMAP_NORMAL);
    *ppy = pos->u.n.py + p->bs;
    lmap_from = pos->from;
    pos = lmap_pfrom(pos->from);
    ((lmap_t *)pos)->u.i.printed = 0;
    lmap_flset(pos->u.i.rf);

    if (syslev == inc_level)
        syslev = -1;

    return p->fptr;
}


/*
 *  checks if the first file is being processed
 */
int (inc_isffile)(void)
{
    return (inc_chain == &incinfo[NELEM(incinfo)-1]);
}

/* end of inc.c */
