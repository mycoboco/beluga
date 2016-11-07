/*
 *  pragma for preprocessing
 */

#include <stddef.h>        /* NULL */
#include <string.h>        /* strcmp */
#include <cbl/assert.h>    /* assert */

#include "lex.h"
#include "lmap.h"
#include "lst.h"
#include "mg.h"
#include "util.h"
#include "prgm.h"

#define match(x) (strcmp(LEX_SPELL(t)+1, (x)+1) == 0)


lex_t *(prgm_start)(lex_t *t, int *rec)
{
    assert(t);
    assert(rec);

    switch(*t->spell) {
        case 'o':    /* once */
            if (match("once")) {
                *rec = 1;
                mg_name = NULL;
                mg_once();
            }
            break;
    }

    NEXTSP(t);    /* consumes last recognized token */
    return t;
}

/* end of prgm.c */
