/*
 *  utilities for preprocessing
 */

#include <stddef.h>        /* size_t */
#include <string.h>        /* strcpy */
#include <cbl/assert.h>    /* assert */
#include <cbl/memory.h>    /* MEM_ALLOC, MEM_RESIZE, MEM_FREE */

#include "lex.h"
#include "util.h"


/*
 *  implements strnlen (which conforms to POSIX.1-2008)
 */
size_t (snlen)(const char *s, size_t max)
{
    const char *t;

    assert(s);

    for (t = s; *(unsigned char *)t && t-s < max; t++)
        continue;

    return t - s;
}


/*
 *  skips whitespaces with variable token input
 */
lex_t *(skip)(lex_t *t, lex_t *(*next)(void))
{
    assert(t);
    assert(next);

    while (t->id == LEX_SPACE)
        t = next();

    return t;
}


/*
 *  accumulates strings managing a buffer for them;
 *  calls to snbuf() must not be interleaved;
 *  used in:
 *    - build() from inc.c to construct full paths;
 *    - concat() from mcr.c to splice tokens and
 *    - derror() from proc.c to collect following tokens
 */
char *(snbuf)(size_t len, int cp)
{
    static char buf[64],    /* size must be power of 2 */
                *p = buf;
    static size_t blen = sizeof(buf);

    assert(p);

    if (len == -1) {
        if (p != buf)
            MEM_FREE(p);
    } else if (len <= blen)
        return p;
    else {
        blen = (len + sizeof(buf)-1) & ~(sizeof(buf)-1);
        if (p == buf) {
            p = MEM_ALLOC(blen);
            if (cp)
                strcpy(p, buf);
        } else
            MEM_RESIZE(p, blen);
    }

    return p;
}

/* end of util.c */
