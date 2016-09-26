/*
 *  line mapper
 */

#include <stddef.h>        /* NULL */
#include <stdio.h>         /* FILE, FOPEN_MAX, fgets, ferror, feof, fopen, fclose, fseek */
#include <string.h>        /* strlen */
#include <cbl/memory.h>    /* MEM_ALLOC, MEM_RESIZE, MEM_FREE */
#include <cbl/arena.h>     /* ARENA_ALLOC */
#include <cbl/assert.h>    /* assert */

#include "common.h"
#include "in.h"
#include "strg.h"
#include "lmap.h"

#define LPUNIT 1024    /* allocation unit for line positions */
#define LBUNIT 256     /* allocation unit for line buffer */


/* line location table */
static struct flb {
    const char *rf;      /* resolved file name */
    sz_t posn;           /* size of pos */
    long *pos;           /* line positions */
    struct flb *link;    /* hash chain */
} *flb[128], *pflb;

/* open file cache */
static struct fpb {
    const char *rf;    /* resolved file name */
    FILE *fp;          /* file pointer */
} fpb[FOPEN_MAX/2];

static char buf[LBUNIT], *pbuf = buf;    /* line buffer */
static sz_t bufn = NELEM(buf);           /* size of line buffer */


const lmap_t *lmap_head;    /* current head */


/*
 *  (line location) sets the current line location table
 */
void (lmap_flset)(const char *rf)
{
    unsigned h;
    struct flb *p;

    assert(rf);

    if (!*rf)
        return;

    h = hashkey(rf, NELEM(flb));
    for (p = flb[h]; p; p = p->link)
        if (p->rf == rf)
            break;

    if (!p) {
        p = ARENA_ALLOC(strg_perm, sizeof(*p));
        p->rf = rf;
        p->posn = LPUNIT;
        p->pos = MEM_ALLOC(LPUNIT * sizeof(*p->pos));
        p->link = flb[h];
        flb[h] = p;
    }

    pflb = p;
}


/*
 *  (line location) remembers a line location
 */
void (lmap_fline)(sz_t py, long pos)
{
    assert(py > 0);

    if (!pflb || pos < 0)
        return;

    if (py > pflb->posn) {
        pflb->posn += LPUNIT;
        MEM_RESIZE(pflb->pos, pflb->posn*sizeof(*pflb->pos));
    }
    pflb->pos[py-1] = pos;
}


/*
 *  (line location) reads a line from a file
 */
static const char *line(FILE *fp)
{
    sz_t len = 0;

    pbuf[0] = '\n', pbuf[1] = '\0';    /* for one past last line */

    while (1) {
        fgets(pbuf+len, bufn-len, fp);
        if (ferror(fp)) {
            rewind(fp);
            return NULL;
        }
        len += strlen(pbuf+len);
        if (pbuf[len-1] == '\n' || feof(fp)) {
            if (pbuf[len-1] == '\n')
                pbuf[len-1] = '\0';
            return pbuf;
        }
        bufn += LBUNIT;
        if (pbuf == buf) {
            pbuf = MEM_ALLOC(bufn);
            strcpy(pbuf, buf);
        } else
            MEM_RESIZE(pbuf, bufn);
    }
}


/*
 *  (line location) gets a source line;
 *  ASSUMPTION: ftell()/fseek() works for different stream of the same file
 */
const char *(lmap_flget)(const char *rf, sz_t py)
{
    unsigned h;
    struct flb *p;
    long pos;
    FILE *fp;

    assert(rf);
    assert(py > 0);

    if (!*rf)
        return NULL;

    h = hashkey(rf, NELEM(flb));
    for (p = flb[h]; p; p = p->link)
        if (p->rf == rf)
            break;
    if (!p || p->posn < py)
        return NULL;
    pos = p->pos[py-1];

    h = hashkey(rf, NELEM(fpb));
    if (fpb[h].rf == rf)
        fp = fpb[h].fp;
    else {
        if ((fp = fopen(rf, "r")) == NULL)
            return NULL;
        if (fpb[h].fp)
           fclose(fpb[h].fp);
        fpb[h].rf = rf;
        fpb[h].fp = fp;
    }

    if (fseek(fp, pos, 0) != 0)
        return NULL;
    return line(fp);
}


/*
 *  (source locus) adds a source locus
 */
lmap_t *(lmap_add)(int dy, sz_t wx)
{
    lmap_t *p = ARENA_ALLOC(strg_perm, sizeof(*p));

    p->type = LMAP_NORMAL;
    p->u.n.py = in_py + dy;
    p->u.n.wx = wx;
    p->u.n.dy = 0;
    p->u.n.dx = wx + 1;
    p->from = lmap_head;

    return p;
}


/*
 *  (source locus) finds a head that has physical file information
 */
const lmap_t *(lmap_getpi)(const lmap_t *p)
{
    for (; LMAP_ISMCR(p); p = p->from)
        continue;

    return p;
}


/*
 *  (source locus) finds a head that has nominal file information
 */
const lmap_t *(lmap_getni)(const lmap_t *p)
{
    for (; LMAP_ISMCR(p); p = p->from)
        continue;

    return p;
}


/*
 *  (source locus) constructs a range from two source loci
 */
const lmap_t *(lmap_range)(const lmap_t *s, const lmap_t *e)
{
    lmap_t *p;

    assert(s);

    if (!e)
        return s;

    while (LMAP_FROMMCR(s))
        s = s->from;
    while (LMAP_FROMMCR(e))
        e = e->from;

    p = ARENA_ALLOC(strg_perm, sizeof(*p));
    p->type = LMAP_NORMAL;
    p->u.n.py = s->u.n.py;
    p->u.n.wx = s->u.n.wx;
    p->u.n.dy = e->u.n.py+e->u.n.dy - s->u.n.py;
    p->u.n.dx = e->u.n.dx;
    p->from = s->from;

    return p;
}


/*
 *  copies a source locus setting the from field to lmap_head
 */
const lmap_t *(lmap_copy)(const lmap_t *s, arena_t *a)
{
    lmap_t *p = ARENA_ALLOC(a, sizeof(*p));

    assert(s);

    memcpy(p, s, sizeof(*p));
    p->from = lmap_head;

    return p;
}


/*
 *  initializes the line mapper
 */
void (lmap_init)(const char *f, const char *rf)
{
    static lmap_t head;

    head.type = -1;
    head.u.i.f = f;
    head.u.i.rf = rf;

    lmap_head = &head;
}


/*
 *  finalizes the line mapper
 */
void (lmap_close)(void)
{
    int i;
    struct flb *p;

    for (i = 0; i < NELEM(flb); i++)
        for (p = flb[i]; p; p = p->link)
            if (p->pos)
                MEM_FREE(p->pos);
    for (i = 0; i < NELEM(fpb); i++)
        if (fpb[i].fp)
            fclose(fpb[i].fp);
    if (pbuf != buf)
        MEM_FREE(pbuf);
}

/* end of lmap.c */
