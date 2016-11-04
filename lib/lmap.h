/*
 *  line mapper
 */

#ifndef LMAP_H
#define LMAP_H

#include <cbl/arena.h>    /* arena_t */
#ifndef NDEBUG
#include <stdio.h>        /* FILE */
#endif    /* !NDEBUG */

#include "common.h"


/* line mapper node type */
enum {
    /* -1 indicates root */
    LMAP_INC,      /* #include */
    LMAP_LINE,     /* #line */
    LMAP_MACRO,    /* macro expansion */
    LMAP_NORMAL    /* normal node; not header */
};

/* line mapper node */
typedef struct lmap_t {
    int type;    /* type */
    union {
        struct {
            const char *f;          /* nominal file name; cis */
            sz_t yoff;              /* added only for cis */
            const char *rf;         /* resolved file name */
            unsigned printed: 1;    /* true if #include chain printed */
            unsigned system:  1;    /* true in system header */
        } i;                        /* LMAP_INC */
        struct {
            const char *f;    /* file name by #line if any; cis */
            sz_t yoff;        /* line # by #line = py+yoff; cis */
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
extern const lmap_t *lmap_cmd;                  /* command line locus */
extern const lmap_t *lmap_bltin;                /* built-in locus */
extern const lmap_t *(*lmap_add)(int, sz_t);    /* function to get source locus */


void lmap_flset(const char *);
void lmap_fline(sz_t, long);
const char *lmap_flget(const char *, sz_t);

void lmap_setadd(int);
const lmap_t *lmap_range(const lmap_t *, const lmap_t *);
const lmap_t *lmap_spell(const lmap_t *, const char *, const char *, const char *, const char *);
const lmap_t *lmap_include(const char *, const char *, const lmap_t *, int);
const lmap_t *lmap_line(const char *, sz_t, const lmap_t *);
const lmap_t *lmap_macro(const lmap_t *, const lmap_t *, arena_t *);

const lmap_t *lmap_npfrom(int, const lmap_t *);
const lmap_t *lmap_mstrip(const lmap_t *);

void lmap_init(const char *, const char *);
void lmap_close(void);

#ifndef NDEBUG
void lmap_print(const lmap_t *, FILE *);
#endif    /* !NDEBUG */


/* makes lex_next() use a generated or dummy locus */
#define lmap_setadd()   (lmap_setadd(0))
#define lmap_clearadd() ((lmap_setadd)(1))

/* find node for nominal or physical information */
#define lmap_nfrom(p) (lmap_npfrom(1, (p)))
#define lmap_pfrom(p) (lmap_npfrom(0, (p)))


#endif    /* LMAP_H */

/* end of lmap.h */
