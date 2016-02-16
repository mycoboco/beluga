/*
 *  IR interface and binding
 */

#include <stddef.h>        /* NULL */
#include <string.h>        /* strcmp */
#include <cbl/assert.h>    /* assert */

#include "ir.h"


/* predefined IR bindings */
extern ir_t ir_bnull;    /* null binding (bnull.c) */
extern ir_t ir_bx86t;    /* lcc's x86-test binding for validation (bx86t.c/r) */
extern ir_t ir_bx86l;    /* x86-linux binding (bx86l.c/r) */


/* IR binding table */
static const struct bind_t {
    const char *name;    /* name for binding */
    ir_t *ir;            /* IR binding */
} binding[] = {
    "null",      &ir_bnull,    /* null */
    "x86-test",  &ir_bx86t,    /* lcc's x86-test for validation */
    "x86-linux", &ir_bx86l     /* x86-linux */
};


ir_t *ir_cur;    /* current IR binding */


/*
 *  finds an IR binding by name
 */
ir_t *(ir_bind)(const char *name)
{
    int i;

    assert(name);

    for (i = 0; i < NELEM(binding); i++)
        if (strcmp(name, binding[i].name) == 0)
            return binding[i].ir;

    return NULL;
}

/* end of ir.c */
