/*
 *  exception (cbl)
 */

#include <stddef.h>    /* NULL */
#include <setjmp.h>    /* longjmp */
#include <stdio.h>     /* stderr, fprintf, fflush */
#include <stdlib.h>    /* abort */

#include "cbl/assert.h"    /* assert with exception support */
#include "except.h"


/* stack for nested exceptions */
except_frame_t *except_stack = NULL;


/*
 *  raises an exception and set its information properly
 *
 *  Note that the current exception frame is popped as soon as possible, which enables another
 *  exception occurred while handling the current exception to be handled by the previous hander.
 *
 *  TODO:
 *    - it would be useful to show stack traces when an uncaught exception leads to abortion of a
 *      program. The stack traces should include as much information as possible, for example,
 *      names of caller functions, calling sites (file name, function name and line number) and
 *      arguments.
 */
#if __STDC_VERSION__ >= 199901L    /* C99 version */
void (except_raise)(const except_t *e, const char *file, const char *func, int line)
#else    /* C90 version */
void (except_raise)(const except_t *e, const char *file, int line)
#endif    /* __STDC_VERSION__ */
{
    except_frame_t *p = except_stack;    /* current exception frame */

    assert(e);
    if (!p) {    /* no current exception frame */
        fprintf(stderr, "Uncaught exception");
        if (e->exception && e->exception[0] != '\0')
            fprintf(stderr, " %s", e->exception);
        else
            fprintf(stderr, " at 0x%p", (void *)e);
#if __STDC_VERSION__ >= 199901L    /* C99 version */
        if (file && func && line > 0)
            fprintf(stderr, " raised at %s() %s:%d\n", func, file, line);
#else    /* C90 version */
        if (file && line > 0)
            fprintf(stderr, " raised at %s:%d\n", file, line);
#endif    /* __STDC_VERSION__ */
        fprintf(stderr, "Aborting...\n");
        fflush(stderr);
        abort();
    } else {
        /* set exception frame properly */
        p->exception = e;
        p->file = file;
#if __STDC_VERSION__ >= 199901L    /* C99 supported */
        p->func = func;
#endif
        p->line = line;

        /* pop exception stack */
        except_stack = except_stack->prev;

        /* exception raised but not handled yet, so EXCEPT_RAISED */
        longjmp(p->env, EXCEPT_RAISED);
    }
}

/* end of except.c */
