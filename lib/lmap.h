/*
 *  line mapper
 */

#ifndef LMAP_H
#define LMAP_H

#include <cbl/arena.h>    /* arena_t */

#include "common.h"


/* line mapper node type */
enum {
    /* -1 indicates root */
    LMAP_IN,       /* #include start */
    LMAP_OUT,      /* #include end */
    LMAP_LINE,     /* #line */
    LMAP_MACRO,    /* macro expansion */
    LMAP_NORMAL    /* normal node; not header */
};

/* line mapper node */
typedef struct lmap_t {
    int type;    /* type */
    union {
        struct {
            const char *rf;    /* resolved file name */
            const char *f;     /* nominal file name */
        } i;                   /* LMAP_IN/OUT */
        struct {
            const char *f;    /* file name by #line if any */
            sz_t yoff;        /* line # by #line = from's py + yoff */
        } l;                  /* LMAP_LINE */
        const struct lmap_t *m;    /* original locus for LMAP_MACRO */
        struct {
            sz_t py;    /* physical y */
            sz_t wx;    /* x counted by wcwidth() */
            int dy;     /* extra lines token occupies */
            sz_t dx;    /* x at which token ends; counted by wcwidth() */
        } n;            /* LMAP_NORMAL */
    } u;
    const struct lmap_t *from;    /* head or originating node */
} lmap_t;


extern const lmap_t *lmap_from;                 /* current from node */
extern const lmap_t *(*lmap_add)(int, sz_t);    /* function to get source locus */


void lmap_flset(const char *);
void lmap_fline(sz_t, long);
const char *lmap_flget(const char *, sz_t);

void lmap_setadd(int);
const lmap_t *lmap_range(const lmap_t *, const lmap_t *);
const lmap_t *lmap_macro(const lmap_t *, const lmap_t *, arena_t *);

void lmap_init(const char *, const char *);
void lmap_close(void);


/* makes lex_next() use a generated or dummy locus */
#define lmap_setadd()   (lmap_setadd(0))
#define lmap_clearadd() ((lmap_setadd)(1))


#endif    /* LMAP_H */

/* end of lmap.h */
