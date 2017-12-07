/*
 *  memory - debugging version (cbl)
 */

#include <stddef.h>    /* NULL, size_t */
#include <stdlib.h>    /* malloc */
#include <string.h>    /* memcpy, memset */
#include <stdio.h>     /* FILE, fflush, fputs, fprintf, stderr */
#if __STDC_VERSION__ >= 199901L    /* C99 supported */
#include <stdint.h>    /* uintptr_t */
#endif    /* __STDC_VERSION__ */

#include "cbl/assert.h"    /* assert with exception support */
#include "cbl/except.h"    /* except_raise, EXCEPT_RAISE */
#include "memory.h"


#define NELEMENT(array) (sizeof(array) / sizeof(*(array)))    /* number of elements in array */

#define HASH(p, t) (((uintptr_t)(p)>>3) % NELEMENT(t))    /* hash from pointer */

#define NDESCRIPTORS 512    /* number of block descriptors; see descalloc() */

/* smallest multiple of y greater than or equal to x */
#define MULTIPLE(x, y) ((((x)+(y)-1)/(y)) * (y))

/* extra bytes allocated with new memory block; see mem_alloc() */
#define NALLOC MULTIPLE(4096, sizeof(union align))

/* checks if pointer aligned properly */
#define ALIGNED(p) ((uintptr_t)(p) % sizeof(union align) == 0)

/* raises exception for invalid pointers */
#if __STDC_VERSION__ >= 199901L    /* C99 version */
#define RAISE_EXCEPT_IF_INVALID(p, n, type)                                             \
            do {                                                                        \
                if (!ALIGNED(p) || (bp = descfind(p)) == NULL || bp->free) {            \
                    if (logfile)                                                        \
                        logprint((p), (n), bp, file, func, line, (int (*)())(type));    \
                    else                                                                \
                        except_raise(&assert_exceptfail, file, func, line);             \
                }                                                                       \
            } while(0)
#else    /* C90 version */
#define RAISE_EXCEPT_IF_INVALID(p, n, type)                                       \
            do {                                                                  \
                if (!ALIGNED(p) || (bp = descfind(p)) == NULL || bp->free) {      \
                    if (logfile)                                                  \
                        logprint((p), (n), bp, file, line, (int (*)())(type));    \
                    else                                                          \
                        except_raise(&assert_exceptfail, file, line);             \
                }                                                                 \
            } while(0)
#endif    /* __STDC_VERSION__ */


#if __STDC_VERSION__ >= 199901L    /* C99 supported */
#    ifndef UINTPTR_MAX    /* C99, but uintptr_t not provided */
#    error "No integer type to contain pointers without loss of information!"
#    endif    /* UINTPTR_MAX */
#else    /* C90, uintptr_t surely not supported */
typedef unsigned long uintptr_t;
#endif    /* __STDC_VERSION__ */

union align {
#ifdef MEM_MAXALIGN
    char pad[MEM_MAXALIGN];
#else    /* guesses maximum alignment requirement */
    int i;
    long l;
    long *lp;
    void *p;
    void (*fp)(void);
    float f;
    double d;
    long double ld;
#endif    /* MEM_MAXALIGN */
};

/* memory block descriptor */
struct descriptor {
    struct descriptor *free;    /* next descriptor for free block; non-null iff freed */
    struct descriptor *link;    /* next descriptor in same hash entry */
    const void *ptr;            /* memory block maintained by descriptor */
    size_t size;                /* size of memory block */
    const char *file;           /* file name in which memory block allocated */
#if __STDC_VERSION__ >= 199901L    /* C99 supported */
    const char *func;           /* function name in which memory block allocated */
#endif    /* C99/C90 version */
    int line;                   /* line number on which memory block allocated */
};


/* exception for memory allocation failure */
const except_t mem_exceptfail = { "Allocation failed" };


/* file for logging invalid memory operation */
static FILE *logfile;

/* user-provided free-free log function */
static void (*logfuncFreefree)(FILE *, const mem_loginfo_t *);

/* user-provided resize-free log function */
static void (*logfuncResizefree)(FILE *, const mem_loginfo_t *);

/*
 *  hash table for memory block descriptors
 *
 *  htab's elements point to lists for block descriptors. htab contains every memory block
 *  descriptor whether it describes a freed or used block. For example, suppose there are 3 entries
 *  in a table and 3 descriptors, each of which has its own allocated memory blocks:
 *
 *      +-+    +-+    +-+
 *      | | -> | | -> | | -> null
 *      +-+    +-+    +-+
 *      | |
 *      +-+    +-+
 *      | | -> | | -> null
 *      +-+    +-+
 *      htab
 *
 *  Now, when one of the memory blocks is freed, the descriptor for that block is marked as "freed"
 *  rather than releasing it. This "freed" block is reused if possible when allocation of a new
 *  memory block is requested. With two memory blocks are freed, we have:
 *
 *      +-+    +-+    +-+
 *      | | -> |F| -> | | -> null
 *      +-+    +-+    +-+
 *      | |
 *      +-+    +-+
 *      | | -> |F| -> null
 *      +-+    +-+
 *      htab
 *
 *  where F indicates "freed". With a separate list to thread the freed blocks, looking for it when
 *  a new block requested can be more efficient, which is what freelist defined below for.
 */
static struct descriptor *htab[2048];

/*
 *  threads descriptors for freed memory blocks
 *
 *  freelist is a tail dummy node of the list for free blocks. Its free member points to the head
 *  node of the list; it points to itself initially as shown below. As explained above, the hash
 *  table for descriptors contains all of the freed and allocated descriptors. freelist threads
 *  only freed blocks and helps to traverse them when memory allocation requested.
 */
static struct descriptor freelist = { &freelist, };


/*
 *  finds a descriptor for a memory block
 */
static struct descriptor *descfind(const void *p)
{
    struct descriptor *bp = htab[HASH(p, htab)];    /* finds hash entry */

    while (bp && bp->ptr != p)
        bp = bp->link;

    return bp;    /* if not found, bp is null here */
}


/*
 *  prints a log message
 */
#if __STDC_VERSION__ >= 199901L    /* C99 version */
static void logprint(void *p, size_t n, struct descriptor *bp, const char *file, const char *func,
                     int line, int (*type)())
#else    /* C90 version */
static void logprint(void *p, size_t n, struct descriptor *bp, const char *file, int line,
                     int (*type)())
#endif    /* __STDC_VERSION__ */
{
    enum {
        TYPE_FREE = 0x10,     /* invoked by mem_free() - free-free case */
        TYPE_RESIZE = 0x20    /* invoked by mem_resize() - resize-free case */
    };
    unsigned int logtype;

    assert(logfile);

    logtype = (type == (int (*)())mem_free)? TYPE_FREE:
              (type == (int (*)())mem_resize)? TYPE_RESIZE: 0;

    logtype |= ((logtype == TYPE_FREE && logfuncFreefree) ||
                (logtype == TYPE_RESIZE && logfuncResizefree))? 1: 0;

    assert(logtype != 0);    /* type should be either mem_free or mem_resize */

    if (logtype & 1) {    /* user print function provided */
        mem_loginfo_t loginfo = { 0, };

        loginfo.p = p;
        loginfo.size = n;
        loginfo.ifile = file;
#if __STDC_VERSION__ >= 199901L    /* C99 version */
        loginfo.ifunc = func;
#endif    /* __STDC_VERSION__ */
        if (file && line > 0)
            loginfo.iline = line;
        if (bp) {
            loginfo.afile = bp->file;
#if __STDC_VERSION__ >= 199901L    /* C99 version */
            loginfo.afunc = bp->func;
#endif    /* __STDC_VERSION__ */
            if (bp->file && bp->line > 0)
                loginfo.aline = bp->line;
            loginfo.asize = bp->size;
        }

        if ((logtype & 0xF0) == TYPE_FREE)
            logfuncFreefree(logfile, &loginfo);
        else    /* (logtype & 0xF0) == TYPE_RESIZE */
            logfuncResizefree(logfile, &loginfo);
    } else {    /* defulat message used */
#if __STDC_VERSION__ >= 199901L    /* C99 version */
        const char *resizefree_msg = "** resizing unallocated memory\n"
                                     "mem_resize(%p, %zu) called from %s() %s:%d\n";
        const char *freefree_msg = "** freeing free memory\n"
                                   "mem_free(%p) called from %s() %s:%d\n";
        const char *bpinfo_msg = "this block is %zu bytes long and was allocated from %s() %s:%d\n";
#else    /* C90 version */
        const char *resizefree_msg = "** resizing unallocated memory\n"
                                     "mem_resize(%p, %lu) called from %s:%d\n";
        const char *freefree_msg = "** freeing free memory\n"
                                   "mem_free(%p) called from %s:%d\n";
        const char *bpinfo_msg = "this block is %lu bytes long and was allocated from %s:%d\n";
#endif    /* __STDC_VERSION__ */

        if (!file) {
            file = "unknown file";
        }
#if __STDC_VERSION__ >= 199901L    /* C99 version */
        if (!func)
            func = "unknown function";
#endif    /* __STDC_VERSION__ */

#if __STDC_VERSION__ >= 199901L    /* C99 version */
        if ((logtype & 0xF0) == TYPE_FREE)
            fprintf(logfile, freefree_msg, p, func, file, line);
        else    /* (logtype & 0xF0) == TYPE_RESIZE */
            fprintf(logfile, resizefree_msg, p, n, func, file, line);
        if (bp && bp->file) {
            const char *bpfunc = (bp->func)? bp->func: "unknow function";
            fprintf(logfile, bpinfo_msg, bp->size, bpfunc, bp->file, bp->line);
        }
#else
        if ((logtype & 0xF0) == TYPE_FREE)
            fprintf(logfile, freefree_msg, p, file, line);
        else    /* (logtype & 0xF0) == TYPE_RESIZE */
            fprintf(logfile, resizefree_msg, p, (unsigned long)n, file, line);
        if (bp && bp->file) {
            fprintf(logfile, bpinfo_msg, (unsigned long)bp->size, bp->file, bp->line);
        }
#endif
        fflush(logfile);
    }
}


/*
 *  deallocates a memory block
 */
#if __STDC_VERSION__ >= 199901L    /* C99 version */
void (mem_free)(void *p, const char *file, const char *func, int line)
#else    /* C90 version */
void (mem_free)(void *p, const char *file, int line)
#endif    /* __STDC_VERSION__ */
{
    if (p) {
        struct descriptor *bp;

        RAISE_EXCEPT_IF_INVALID(p, 0, mem_free);
        bp->free = freelist.free;    /* pushes to free list */
        freelist.free = bp;
    }
}


/*
 *  adjusts the size of a memory block
 */
#if __STDC_VERSION__ >= 199901L    /* C99 version */
void *(mem_resize)(void *p, size_t n, const char *file, const char *func, int line)
#else    /* C90 version */
void *(mem_resize)(void *p, size_t n, const char *file, int line)
#endif    /* __STDC_VERSION__ */
{
    struct descriptor *bp;
    void *np;

    assert(p);
    assert(n > 0);

    RAISE_EXCEPT_IF_INVALID(p, n, mem_resize);
#if __STDC_VERSION__ >= 199901L    /* C99 version */
    np = mem_alloc(n, file, func, line);
    memcpy(np, p, (n < bp->size)? n: bp->size);
    mem_free(p, file, func, line);
#else    /* C90 version */
    np = mem_alloc(n, file, line);
    memcpy(np, p, (n < bp->size)? n: bp->size);
    mem_free(p, file, line);
#endif    /* __STDC_VERSION__ */

    return np;
}


/*
 *  allocates a zero-filled memory block
 *
 *  TODO:
 *    - the C standard requires calloc() return a null pointer if it can allocates no storage of
 *      the size c * n in bytes, which allows no overflow in computing the multiplication. Overflow
 *      checking is necessary to mimic the behavior of calloc()
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

#if __STDC_VERSION__ >= 199901L    /* C99 version */
    p = mem_alloc(c*n, file, func, line);
#else    /* C90 version */
    p = mem_alloc(c*n, file, line);
#endif    /* __STDC_VERSION__ */
    memset(p, '\0', c*n);

    return p;
}


/*
 *  returns an available descriptor
 *
 *  The storage for descriptors allocated by descalloc() is never deallocated. For example, when
 *  512 descriptors had been consumed and there is a request for new one, descalloc() gives up
 *  tracking of the former 512 descriptors and allocates new 512 descriptors to avail. This surely
 *  causes memory leak, but is not a big deal because the purpose of the library is to debug. A
 *  tool to check memory leak like Valgrind would report that descalloc() does not return to an OS
 *  storages it allocated, but it is intentional and never means that a program in debugging with
 *  the library has a bug.
 */
#if __STDC_VERSION__ >= 199901L    /* C99 version */
static struct descriptor *descalloc(void *p, size_t n, const char *file, const char *func, int line)
#else    /* C90 version */
static struct descriptor *descalloc(void *p, size_t n, const char *file, int line)
#endif    /* __STDC_VERSION__ */
{
    static struct descriptor *avail;    /* array of available descriptors */
    static int nleft;    /* number of descriptors left in pool */

    if (nleft <= 0) {    /* additional allocation for descriptors */
        avail = malloc(NDESCRIPTORS * sizeof(*avail));
        if (!avail)
            return NULL;
        nleft = NDESCRIPTORS;
    }

    /* fills up descriptor */
    avail->ptr = p;
    avail->size = n;
    avail->file = file;
#if __STDC_VERSION__ >= 199901L    /* C99 version */
    avail->func = func;
#endif    /* __STDC_VERSION__ */
    avail->line = line;
    avail->free = avail->link = NULL;

    nleft--;

    return avail++;    /* prepare to return next descriptor on next request */
}


/*
 *  allocates a new memory block
 *
 *  The debugging version of mem_alloc() first tries to find a memory block whose size is greater
 *  than n in the free list maintained by both the hash table htab and freelist. If there is none,
 *  it allocates a new block by invoking malloc() and pushes it to the free list to make it a sure
 *  find, and then gives it a shot again.
 *
 *  One of most important things in this debugging version is that mem_alloc() must not return an
 *  address that has been returned by a previous call. To achieve this, mem_alloc() looks for a
 *  memory block whose size is *greater* than n, therefore even if there is one of the exactly same
 *  size as n, it is left unused; it is enough to guarantee at least one byte whose address has
 *  been returned before not to be returned again. Besides, if the found block is large enough to
 *  satisfy the specified size n, it is split and the bottom part is returned as the result
 *  preserving the top address (which has been returned before) in the list. When such split
 *  occurs, a new descriptor given by descalloc() is assigned to that new block.
 *
 *  Differently from the original code, this implementation checks with union align if the storage
 *  allocated by malloc() is properly aligned and makes assert() fail if not. This signals to
 *  define MEM_MAXALIGN proplery to match the real alignment restriction of an implementation in
 *  use. If this alarm ignored, a program using this library is not guaranteed to work.
 */
#if __STDC_VERSION__ >= 199901L    /* C99 version */
void *(mem_alloc)(size_t n, const char *file, const char *func, int line)
#else    /* C90 version */
void *(mem_alloc)(size_t n, const char *file, int line)
#endif    /* __STDC_VERSION__ */
{
    struct descriptor *bp;
    void *p;

    assert(n > 0);

    n = MULTIPLE(n, sizeof(union align));    /* n adjusted here, thus at least MULTIPLE(n, align)
                                                bytes never reused */

    for (bp = freelist.free; bp; bp = bp->free) {
        if (bp->size > n) {    /* enough large block found - first fit;
                                  note equality comparison is excluded */
            bp->size -= n;
            p = (char *)bp->ptr + bp->size;    /* allocates from bottom up */
#if __STDC_VERSION__ >= 199901L    /* C99 version */
            if ((bp = descalloc(p, n, file, func, line)) != NULL) {
#else    /* C90 version */
            if ((bp = descalloc(p, n, file, line)) != NULL) {
#endif    /* __STDC_VERSION__ */
                /* pushes to hash table and return */
                unsigned h = HASH(p, htab);
                bp->link = htab[h];
                htab[h] = bp;
                return p;
            } else {    /* descriptor allocation failed */
                if (!file)
                    EXCEPT_RAISE(mem_exceptfail);
                else
#if __STDC_VERSION__ >= 199901L    /* C99 version */
                    except_raise(&mem_exceptfail, file, func, line);
#else    /* C90 version */
                    except_raise(&mem_exceptfail, file, line);
#endif    /* __STDC_VERSION__ */
            }
        }

        if (bp == &freelist) {    /* proper block not found in free list */
            struct descriptor *np;
            if ((p = malloc(n + NALLOC)) == NULL ||
#if __STDC_VERSION__ >= 199901L    /* C99 version */
                (np = descalloc(p, n+NALLOC, __FILE__, __func__, __LINE__)) == NULL) {
#else    /* C90 version */
                (np = descalloc(p, n+NALLOC, __FILE__, __LINE__)) == NULL) {
#endif    /* __STDC_VERSION__ */
                free(p);
                if (!file)
                    EXCEPT_RAISE(mem_exceptfail);
                else
#if __STDC_VERSION__ >= 199901L    /* C99 version */
                    except_raise(&mem_exceptfail, file, func, line);
#else    /* C90 version */
                    except_raise(&mem_exceptfail, file, line);
#endif    /* __STDC_VERSION__ */
            }
            /* checks if guess at alignment restriction holds */
            assert(ALIGNED(p));    /* if fails, define MEM_MAXALIGN properly */
            /* pushes new block to free list to make it sure find and repeat */
            np->free = freelist.free;
            freelist.free = np;
        }
    }
    assert(!"bp is null - should never reach here");

    return NULL;
}


/*
 *  starts to log invalid memory uses
 */
void (mem_log)(FILE *fp, void freefunc(FILE *, const mem_loginfo_t *),
               void resizefunc(FILE *, const mem_loginfo_t *))
{
    logfile = fp;
    logfuncFreefree = freefunc;
    logfuncResizefree = resizefunc;
}


/*
 *  prints the information for memory leak to a file
 */
static void leakinuse(const mem_loginfo_t *loginfo, void *cl)
{
    FILE *log = (cl)? cl:
                (logfile)? logfile: stderr;
#if __STDC_VERSION__ >= 199901L    /* C99 version */
    const char *file = (loginfo->afile)? loginfo->afile: "unknown file",
               *func = (loginfo->afunc)? loginfo->afunc: "unknown function";
    const char *msg = "** memory in use at %p\n"
                      "this block is %zu bytes long and was allocated from %s() %s:%d\n";

    fprintf(log, msg, loginfo->p, loginfo->size, func, file, loginfo->aline);
#else    /* C90 version */
    const char *file = (loginfo->afile)? loginfo->afile: "unknown file";
    const char *msg = "** memory in use at %p\n"
                      "this block is %ld bytes long and was allocated from %s:%d\n";

    fprintf(log, msg, loginfo->p, (unsigned long)loginfo->size, file, loginfo->aline);
#endif    /* __STDC_VERSION__ */

    fflush(log);
}


/*
 *  calls a user-provided function for each of memory blocks in use
 */
void (mem_leak)(void apply(const mem_loginfo_t *, void *), void *cl)
{
    size_t i;
    struct descriptor *bp;
    mem_loginfo_t loginfo = { 0, };

    for (i = 0; i < NELEMENT(htab); i++)
        for (bp = htab[i]; bp; bp = bp->link)
            if (!bp->free) {    /* in use */
                loginfo.p = bp->ptr;
                loginfo.size = bp->size;
                loginfo.afile = bp->file;
#if __STDC_VERSION__ >= 199901L    /* C99 version */
                loginfo.afunc = bp->func;
#endif    /* __STDC_VERSION__ */
                if (bp->file && bp->line > 0)
                    loginfo.aline = bp->line;
                if (apply == NULL)
                    leakinuse(&loginfo, cl);
                else
                    apply(&loginfo, cl);
            }
}

/* end of memoryd.c */
