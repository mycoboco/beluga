/*
 *  line mapper
 */

#ifndef LMAP_H
#define LMAP_H

#include "common.h"


/* line mapper node type */
enum {
    /* -1 indicates root */
    LMAP_IN,        /* #include start */
    LMAP_OUT,       /* #include end */
    LMAP_LINE,      /* #line */
    LMAP_STR,       /* stringization */
    LMAP_PASTE,     /* token paste */
    LMAP_NORMAL     /* normal node; not header */
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
        struct {
            sz_t py;    /* physical y */
            sz_t wx;    /* x counted by wcwidth() */
            int dy;     /* extra lines token occupies */
            sz_t dx;    /* x at which token ends; counted by wcwidth() */
        } n;            /* LMAP_NORMAL */
    } u;
    const struct lmap_t *from;    /* head or originating node */
} lmap_t;


extern const lmap_t *lmap_head;    /* current head */


void lmap_flset(const char *);
void lmap_fline(sz_t, long);
const char *lmap_flget(const char *, sz_t);
lmap_t *lmap_add(int, sz_t);

const lmap_t *lmap_getpi(const lmap_t *);
const lmap_t *lmap_getni(const lmap_t *);
const lmap_t *lmap_range(const lmap_t *, const lmap_t *);
const lmap_t *lmap_copy(const lmap_t *);

void lmap_init(const char *, const char *);
void lmap_close(void);


/* checks if a locus denotes or is from macro expansion */
#define LMAP_ISMCR(p)   ((p)->type > LMAP_LINE)
#define LMAP_FROMMCR(p) ((p)->from->type > LMAP_LINE)


#endif    /* LMAP_H */

/* end of lmap.h */
