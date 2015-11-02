/*
 *  memory (cbl)
 */

#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>    /* size_t */
#include <stdio.h>     /* FILE */

#include "cbl/except.h"    /* except_t */


/* info for invalid memory operation */
typedef struct mem_loginfo_t {
    const void *p;        /* pointer value in invalid operation */
    size_t size;          /* requested size; meaningful with mem_resize() */
    const char *ifile;    /* file name for invalid operation */
    const char *ifunc;    /* function name for invalid operation */
    int iline;            /* line number for invalid operation */
    const char *afile;    /* file name for allocation */
    const char *afunc;    /* function name for allocation */
    int aline;            /* line number for allocation */
    size_t asize;         /* size of storage for allocation */
} mem_loginfo_t;


/* exception for memory allocation failure */
extern const except_t mem_exceptfail;


#if __STDC_VERSION__ >= 199901L    /* C99 version */
void *mem_alloc(size_t, const char *, const char *, int);
void *mem_calloc(size_t, size_t, const char *, const char *, int);
void mem_free(void *, const char *, const char *, int);
void *mem_resize(void *, size_t, const char *, const char *, int);
#else    /* C90 version */
void *mem_alloc(size_t, const char *, int);
void *mem_calloc(size_t, size_t, const char *, int);
void mem_free(void *, const char *, int);
void *mem_resize(void *, size_t, const char *, int);
#endif    /* __STDC_VERSION__ */
void mem_log(FILE *, void (FILE *, const mem_loginfo_t *), void (FILE *, const mem_loginfo_t *));
void mem_leak(void (const mem_loginfo_t *, void *), void *);


#if __STDC_VERSION__ >= 199901L    /* C99 version */
#define MEM_ALLOC(n)     (mem_alloc((n), __FILE__, __func__, __LINE__))
#define MEM_CALLOC(c, n) (mem_calloc((c), (n), __FILE__, __func__, __LINE__))
#else    /* C90 version */
#define MEM_ALLOC(n)     (mem_alloc((n), __FILE__, __LINE__))
#define MEM_CALLOC(c, n) (mem_calloc((c), (n), __FILE__, __LINE__))
#endif    /* __STDC_VERSION__ */

/* allocates storage */
#define MEM_NEW(p) ((void)((p) = MEM_ALLOC(sizeof *(p))))
#define MEM_NEW0(p) ((void)((p) = MEM_CALLOC(1, sizeof *(p))))

/* deallocates or resize storage */
#if __STDC_VERSION__ >= 199901L    /* C99 version */
#define MEM_FREE(p) ((void)(mem_free((p), __FILE__, __func__, __LINE__), (p)=0))
#define MEM_RESIZE(p, n) ((p) = mem_resize((p), (n), __FILE__, __func__, __LINE__))
#else    /* C90 version */
#define MEM_FREE(p) ((void)(mem_free((p), __FILE__, __LINE__), (p)=0))
#define MEM_RESIZE(p, n) ((p) = mem_resize((p), (n), __FILE__, __LINE__))
#endif    /* __STDC_VERSION__ */


#endif    /* MEMORY_H */

/* end of memory.h */
