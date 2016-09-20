/*
 *  preprocessor
 */

#include <stdio.h>
#include <cbl/memory.h>

#include "lex.h"
#include "lst.h"
#include "proc.h"
#include "cpp.h"


/*
 *  drives preprocessing
 */
void (cpp_start)(void)
{
    lex_t *t;

    proc_prep();
    while ((t = lst_next())->id != LEX_EOI) {
        if (t->id == LEX_MCR) {
            MEM_FREE(t);
            continue;
        }
        printf("%s", t->spell);
        LEX_FREE(t);
    }
    MEM_FREE(t);    /* frees EOI */
}

/* end of cpp.c */
