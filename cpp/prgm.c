/*
 *  pragma for preprocessing
 */

#include <stddef.h>        /* NULL */
#include <string.h>        /* strcmp */
#include <cbl/assert.h>    /* assert */

#include "lex.h"
#include "lxl.h"
#include "mg.h"
#include "util.h"
#include "prgm.h"

#define match(x) (strcmp(t->rep+1, (x)+1) == 0)


lex_t *(prgm_start)(lex_t *t, int *rec)
{
    assert(t);
    assert(rec);

    switch(*t->rep) {
        case 'o':    /* once */
            if (match("once")) {
                *rec = 1;
                mg_name = NULL;
                mg_once();
            }
    }

    return skip(lxl_next(), lxl_next);
}

/* end of prgm.c */
