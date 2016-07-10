/*
 *  line mapper
 */

#ifndef LMAP_H
#define LMAP_H


/* line mapper node type */
enum {
    /* -1 indicates root */
    LMAP_IN,        /* #include start */
    LMAP_OUT,       /* #include end */
    LMAP_LINE,      /* #line */
    LMAP_MCRS,      /* macro expansion start */
    LMAP_MCRE,      /* macro expansion end */
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
            long yoff;        /* line # by #line = from's py + yoff */
        } l;                  /* LMAP_LINE */
        const char *m;    /* macro name for LMAP_MCRS/E */
        struct {
            long py;    /* physical y */
            int wx;     /* x counted by wcwidth() */
            int n;      /* token length */
        } n;            /* LMAP_NORMAL */
    } u;
    const struct lmap_t *from;    /* head or originating node */
} lmap_t;


const lmap_t *lmap_head;    /* current head */


void lmap_flset(const char *);
void lmap_fline(long, long);
const char *lmap_flget(const char *, long);
const lmap_t *lmap_add(long, int);
const lmap_t *lmap_getpi(const lmap_t *);
const lmap_t *lmap_getni(const lmap_t *);
void lmap_init(const char *, const char *);
void lmap_close(void);


#endif    /* LMAP_H */

/* end of lmap.h */
