/*
 *  stop set
 */

#include <cbl/assert.h>    /* assert */

#include "clx.h"
#include "err.h"
#include "lex.h"
#include "lmap.h"

#define FIRST_EXPR LEX_ID                  /* FIRST(expr) */
#define FIRST_STMT LEX_IF                  /* FIRST(stmt) - FIRST(expr) */
#define FIRST_SPEC LEX_CHAR                /* FIRST_DECL - storage class */
#define FIRST_DECL LEX_CHAR, LEX_STATIC    /* FIRST(decl) - typedef id */


/* predefined stop sets to handle syntax errors */
const char sset_field[] =    { FIRST_STMT, FIRST_SPEC, '}', 0 };
const char sset_strdef[] =   { FIRST_STMT, ',', 0 };
const char sset_enumdef[] =  { FIRST_STMT, 0 };
const char sset_decl[] =     { FIRST_DECL, ';', 0 };
const char sset_declf[] =    { FIRST_DECL, FIRST_EXPR, 0 };
const char sset_declb[] =    { FIRST_STMT, FIRST_DECL, FIRST_EXPR, 0 };
const char sset_expr[] =     { FIRST_STMT, FIRST_EXPR, '}', 0 };
const char sset_exprasgn[] = { FIRST_STMT, FIRST_EXPR, 0 };
const char sset_initf[] =    { FIRST_DECL, ';', 0 };
const char sset_initb[] =    { FIRST_STMT, FIRST_DECL, 0 };



/*
 *  skips tokens until an expected one appears
 */
void (sset_skip)(int tc, const char set[])
{
    const char *s;

    assert(set);

    for (; clx_tc != LEX_EOI && clx_tc != tc; clx_tc = clx_next())
        for (s = set; *s; s++)
            if (clx_kind[clx_tc] == *s || clx_tc == *s)
                return;
}


/*
 *  presumes that an expected token exists
 */
const lmap_t *(sset_expect)(int tc, const lmap_t *posm)
{
    const char *m;

    switch(tc) {
        case ')':
            m = "(";
            break;
        case '}':
            m = "{";
            break;
        case ']':
            m = "[";
            break;
        case ':':
            m = "?";
            break;
        default:
            assert(!posm);
            break;
    }

    if (clx_tc == tc) {
        posm = clx_cpos;
        clx_tc = clx_next();
    } else {
        (void)(err_dpos(CLX_PCPOS(), ERR_PARSE_ERROR, tc, clx_tc) &&
               err_dpos(posm, ERR_PARSE_TOMATCH, m));
        posm = NULL;
    }

    return posm;
}


/*
 *  tests if the next token is tc
 */
void (sset_test)(int tc, const char set[], const lmap_t *posm)
{
    if (clx_tc != tc) {
        sset_expect(tc, posm);
        sset_skip(tc, set);
    }
    if (clx_tc == tc)
        clx_tc = clx_next();
}

/* end of sset.c */
