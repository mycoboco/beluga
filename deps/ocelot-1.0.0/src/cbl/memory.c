/*
 *  memory - production version (cbl)
 */

#include <stddef.h>    /* size_t */
#include <stdlib.h>    /* malloc, calloc, realloc, free */

#include "cbl/assert.h"    /* assert with exception support */
#include "cbl/except.h"    /* EXCEPT_RAISE, except_raise */
#include "memory.h"

#define UNUSED(id) ((void)(id))


/* exception for memory allocation failure */
const except_t mem_exceptfail = { "Allocation failed" };


/*
 *  allocates storage of the size n in bytes
 */
#if __STDC_VERSION__ >= 199901L    /* C99 version */
void *(mem_alloc)(size_t n, const char *file, const char *func, int line)
#else    /* C90 version */
void *(mem_alloc)(size_t n, const char *file, int line)
#endif    /* __STDC_VERSION__ */
{
    void *p;

    assert(n > 0);    /* precludes zero-sized allocation */

    p = malloc(n);
    if (!p)
    {
        if (!file)
            EXCEPT_RAISE(mem_exceptfail);
        else
#if __STDC_VERSION__ >= 199901L    /* C99 version */
            except_raise(&mem_exceptfail, file, func, line);
#else    /* C90 version */
            except_raise(&mem_exceptfail, file, line);
#endif    /* __STDC_VERSION__ */
    }

    return p;
}


/*
 *  allocates zero-filled storage of the size c * n in bytes
 */
#if __STDC_VERSION__ >= 199901L    /* C99 version */
void *(mem_calloc)(size_t c, size_t n, const char *file, const char *func, int line)
#else    /* C90 version */
void *(mem_calloc)(size_t c, size_t n, const char *file, int line)
#endif    /* __STDC_VERSION__ */
{
    void *p;

    assert(c > 0);    /* precludes zero-sized (de)allocation */
    assert(n > 0);

    p = calloc(c, n);
    if (!p)
    {
        if (!file)
            EXCEPT_RAISE(mem_exceptfail);
        else
#if __STDC_VERSION__ >= 199901L    /* C99 version */
            except_raise(&mem_exceptfail, file, func, line);
#else    /* C90 version */
            except_raise(&mem_exceptfail, file, line);
#endif    /* __STDC_VERSION__ */
    }

    return p;
}


/*
 *  deallocates storage
 */
#if __STDC_VERSION__ >= 199901L    /* C99 version */
void (mem_free)(void *p, const char *file, const char *func, int line)
#else    /* C90 version */
void (mem_free)(void *p, const char *file, int line)
#endif    /* __STDC_VERSION__ */
{
    UNUSED(file);
#if __STDC_VERSION__ >= 199901L    /* C99 version */
    UNUSED(func);
#endif    /* __STDC_VERSION__ */
    UNUSED(line);

    /* no need to test if p is null pointer */
    free(p);
}


/*
 *  adjust the size of storage
 */
#if __STDC_VERSION__ >= 199901L    /* C99 version */
void *(mem_resize)(void *p, size_t n, const char *file, const char *func, int line)
#else    /* C90 version */
void *(mem_resize)(void *p, size_t n, const char *file, int line)
#endif    /* __STDC_VERSION__ */
{
    assert(p);
    assert(n > 0);    /* precludes zero-sized allocation */

    p = realloc(p, n);
    if (!p)
    {
        if (!file)
            EXCEPT_RAISE(mem_exceptfail);
        else
#if __STDC_VERSION__ >= 199901L    /* C99 version */
            except_raise(&mem_exceptfail, file, func, line);
#else    /* C90 version */
            except_raise(&mem_exceptfail, file, line);
#endif    /* __STDC_VERSION__ */
    }

    return p;
}


/*
 *  provides a dummy function for mem_log() that is activated in debugging version
 */
void (mem_log)(FILE *fp, void freefunc(FILE *, const mem_loginfo_t *),
             void resizefunc(FILE *, const mem_loginfo_t *))
{
    UNUSED(fp);
    UNUSED(freefunc);
    UNUSED(resizefunc);

    /* do nothing in production version */
}


/*
 *  provides a dummy function for mem_leak() that is activated in debugging version
 */
void (mem_leak)(void apply(const mem_loginfo_t *, void *), void *cl)
{
    UNUSED(apply);
    UNUSED(cl);

    /* do nothing in production version */
}

/* end of memory.c */
