/*
 *  include for preprocessing
 */

#include <stddef.h>        /* NULL, size_t */
#include <stdio.h>         /* FILE, fopen, fclose */
#include <string.h>        /* strlen, strcpy, strrchr, strtok */
#include <cbl/assert.h>    /* assert */
#include <cbl/arena.h>     /* ARENA_ALLOC */
#include <cbl/memory.h>    /* MEM_ALLOC, MEM_FREE */
#include <cdsl/hash.h>     /* hash_new, hash_string */
#include <cdsl/list.h>     /* list_t, list_push, list_reverse, list_free, LIST_FOREACH */

#include "../src/alist.h"
#include "../src/common.h"
#include "../src/err.h"
#include "../src/in.h"
#include "cond.h"
#include "ctx.h"
#include "lex.h"
#include "lxl.h"
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

/* system header directory */
#ifndef SYSTEM_HEADER_DIR
#define SYSTEM_HEADER_DIR /usr/include:/usr/local/include
#endif    /* !SYSTEM_HEADER_DIR */


/* marks end of inc_list */
static inc_t sentinel, *psentinel = &sentinel;


/* #include list;
   initialized because it may be used before inc_init() */
inc_t **inc_list = &psentinel;


static list_t *rplist;              /* raw path list */
static void **path;                 /* (const char *) array of #include paths */
static const char *cwd = ".";       /* current working directory */
static int level;                   /* nesting level of #include's */
static inc_t *incinfo[TL_INC+1];    /* #include list */


/*
 *  adds raw #include paths to parse later
 */
void (inc_add)(const char *s)
{
    char *t;

    t = MEM_ALLOC(strlen(s)+1);    /* will be used with strtok() */
    strcpy(t, s);
    rplist = list_push(rplist, t);
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
        return hash_string(h);

    return hash_new(h, p - h);
}


/*
 *  prepares to process #includes;
 *  no duplication check
 */
void (inc_init)(void)
{
    list_t *p;
    alist_t *list;

    assert(in_cpos.ff);

    list = alist_append(NULL, "", strg_line);
    inc_add(xstr(SYSTEM_HEADER_DIR));
    rplist = list_reverse(rplist);
    LIST_FOREACH(p, rplist) {
        char *s;
        for (s = p->data; (s=strtok(s, PSEP)) != NULL; s = NULL)
            list = alist_append(list, s, strg_line);
    }
    path = alist_toarray(list, strg_perm);

    if (strrchr(in_cpos.ff, DSEP))
        cwd = getcwd(in_cpos.ff);

    incinfo[NELEM(incinfo)-1] = &sentinel;
    inc_list = &incinfo[NELEM(incinfo)-1];
}


/*
 *  frees storages for handling #include's
 */
void (inc_free)(void)
{
    list_t *p;

    LIST_FOREACH(p, rplist) {
        MEM_FREE(p->data);
    }
    list_free(&rplist);
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
    assert(pn);
    assert(*p);
    assert(*h);
    assert(DSEP != '\0');

    np = strlen(p);
    nh = strlen(h);
    full = snbuf(np+nh+2, 0);    /* +2 for DSEP and NUL */

    if (h[0] != DSEP) {
        strcpy(full, p);
        if (p[np - 1] != DSEP) {
            full[np++] = DSEP,
            full[np] = '\0';
        }
    } else
        np = 0;
    strcpy(full+np, h);
    full[np + nh - 1] = '\0';    /* -1 to remove " or > */
    *pn = np;

    return full;
}


/*
 *  recognizes header names
 */
int (inc_start)(const char *fn, const lex_pos_t *ppos)
{
    int q;
    FILE *fp;
    void **p;
    size_t n;
    const char *ffn;

    assert(fn);
    assert(ppos);
    assert(*fn == '<' || *fn == '"');

    q = (*fn++ == '"');
    /* closing character will be deleted later */

    assert(path && *path);
    for (p = path; *p; p++) {
        if (((char *)*p)[0] == '\0') {
            if (!q)
                continue;
            ffn = build(cwd, fn, &n);
        } else
            ffn = build(*p, fn, &n);
        if ((fp = fopen(ffn, "r")) != NULL)
            break;
    }
    if (!*p)
        err_issuep(ppos, ERR_PP_NOINCFILE, ffn + n);

    if (level == TL_INC_STD) {
        err_issuep(ppos, ERR_PP_MANYINC2);
        err_issuep(ppos, ERR_PP_MANYINCSTD, (long)TL_INC_STD);
    }
    if (level == TL_INC)
        err_issuep(ppos, ERR_PP_MANYINC1);
    else {
        in_switch(fp, hash_string((main_opt()->parsable)? ffn: ffn + n));
        assert(!ctx_cur->cur->next);    /* no looked-ahead tokens here */
        cwd = getcwd(ffn);
    }

    return 1;
}


/*
 *  pushes the current context into the #include list
 */
void (inc_push)(FILE *fp)
{
    inc_t *p;

    assert(inc_list > &incinfo[0]);

    level++;
    (*inc_list)->printed = 0;
    if (!*--inc_list)
        *inc_list = ARENA_ALLOC(strg_perm, sizeof(**inc_list));
    p = *inc_list;

    p->fptr = fp;
    p->f = in_cpos.f;
    p->y = in_cpos.y;
    p->mf = in_cpos.mf;
    p->my = in_cpos.my;
    p->limit = in_limit;
    p->line = in_line;
    p->cp = in_cp;
    p->cwd = cwd;
    p->printed = 0;
    p->cond = cond_list;
    cond_list = NULL;
}


/*
 *  pops a context from the #include list
 */
FILE *(inc_pop)(FILE *fp)
{
    inc_t *p;

    assert(fp);
    assert(level > 0);
    assert(inc_list < &incinfo[NELEM(incinfo)-1]);

    level--;
    fclose(fp);

    p = *inc_list++;
    in_cpos.f = p->f;
    in_cpos.y = p->y;
    in_cpos.mf = p->mf;
    in_cpos.my = p->my;
    in_limit = p->limit;
    in_line = p->line;
    in_cp = p->cp;
    cwd = p->cwd;
    cond_list = p->cond;

    return p->fptr;
}


/*
 *  checks if the first file is being processed
 */
int (inc_isffile)(void)
{
    return (inc_list == &incinfo[NELEM(incinfo)-1]);
}

/* end of inc.c */
