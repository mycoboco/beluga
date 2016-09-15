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
    void *p;

    proc_prep();
    while ((t = lst_next())->id != LEX_EOI) {
        if (t->id == LEX_MCR) {
            MEM_FREE(t);
            continue;
        }
        printf("%s", t->spell);
        if (t->f.alloc) {
            p = (void *)t->spell;
            MEM_FREE(p);
        }
        MEM_FREE(t);
    }
    MEM_FREE(t);    /* frees EOI */
}

/* end of cpp.c */
