/*
 *  preprocessor
 */

#include <stdio.h>    /* fputs, stdout */

#include "lex.h"
#include "lst.h"
#include "proc.h"
#include "strg.h"
#include "cpp.h"


/*
 *  drives preprocessing
 */
void (cpp_start)(void)
{
    lex_t *t;

    proc_prep();
    while ((t = lst_next())->id != LEX_EOI) {
        if (t->id < 0) {
            strg_free((arena_t *)t->spell);
            continue;
        } else if (t->id == LEX_MCR)
            continue;
        fputs(LEX_SPELL(t), stdout);
    }
}

/* end of cpp.c */
