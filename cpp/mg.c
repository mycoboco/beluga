/*
 *  macro guard optimization
 */

#include <cbl/assert.h>    /* assert */

#include "../src/common.h"
#include "inc.h"
#include "mcr.h"
#include "strg.h"
#include "mg.h"


/* macro guard table */
struct mgt {
    const char *path;    /* #include file path */
    const char *name;    /* macro for #include guard */
    struct mgt *link;    /* hash chain */
} *mgt[128];


int mg_state = MG_SINIT;    /* macro guard state */
const char *mg_name;        /* macro for #inlude guard */


/*
 *  remembers a macro guard
 */
void (mg_once)(void)
{
    unsigned h;
    struct mgt *p;
    const char *path = INC_REALPATH(inc_fpath);

    h = hashkey(path, NELEM(mgt));
    for (p = mgt[h]; p; p = p->link)
        if (p->path == path) {
            if (p->name)    /* #pragma once always wins */
                p->name = mg_name;
            return;
        }

    p = ARENA_ALLOC(strg_perm, sizeof(*p));
    p->path = path;
    p->name = mg_name;
    p->link = mgt[h];
    mgt[h] = p;
}


/*
 *  checks if a file is macro-guarded;
 *  path is assumed to be a hash string
 */
int (mg_isguarded)(const char *path)
{
    unsigned h;
    struct mgt *p;

    assert(path);

    h = hashkey(path, NELEM(mgt));
    for (p = mgt[h]; p; p = p->link)
        if (p->path == path && (!p->name || mcr_redef(p->name)))
            return 1;

    return 0;
}

/* end of mg.c */
