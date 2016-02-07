/*
 *  conditional inclusion for preprocessing
 */

#include <cbl/assert.h>    /* assert */
#include <cbl/memory.h>    /* MEM_NEW, MEM_FREE */

#include "../src/common.h"
#include "../src/err.h"
#include "lex.h"
#include "cond.h"


cond_t *cond_list;    /* conditional list */


/*
 *  pushes the current context into the conditional list
 */
void (cond_push)(int kind, const lex_pos_t *ppos)
{
    cond_t *p;

    assert(ppos);

    MEM_NEW(p);

    if (cond_list) {
        if (cond_list->level == TL_COND_STD) {
            err_issuep(ppos, ERR_PP_MANYCOND);
            err_issuep(ppos, ERR_PP_MANYCONDSTD, (long)TL_COND_STD);
        }
        p->level = cond_list->level + 1;
    } else
        p->level = 0;

    p->kind = kind;
    p->f.once = 0;
    p->f.ignore = 0;
    p->ifpos = *ppos;
    p->elsepos.g.y = 0;
    p->prev = cond_list;
    cond_list = p;
}


/*
 *  pops a context from the conditional list
 */
void (cond_pop)(void)
{
    cond_t *p;

    assert(cond_list);

    p = cond_list;
    cond_list = cond_list->prev;
    MEM_FREE(p);
}


/*
 *  checks if there is any unterminated #if/#ifdef/#ifndef
 */
void (cond_finalize)(void)
{
    while (cond_list) {
        err_issuep(&cond_list->ifpos, ERR_PP_UNTERMCOND, cond_list->kind);
        cond_pop();
    }

    assert(!cond_list);
}


/*
 *  returns a name for conditional kind
 */
const char *(cond_name)(int kind)
{
    static const char *s[] = {
        "#if",
        "#ifdef",
        "#ifndef"
    };

    assert(kind >= 0);
    assert(kind < NELEM(s));

    return s[kind];
}

/* end of cond.c */
