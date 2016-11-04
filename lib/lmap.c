/*
 *  line mapper
 */

#include <stddef.h>        /* NULL */
#include <stdio.h>         /* FILE, FOPEN_MAX, fgets, ferror, feof, fopen, fclose, fseek */
#include <string.h>        /* strlen */
#include <cbl/memory.h>    /* MEM_ALLOC, MEM_RESIZE, MEM_FREE */
#include <cbl/arena.h>     /* ARENA_ALLOC */
#include <cbl/assert.h>    /* assert */
#ifndef NDEBUG
#include <stdio.h>         /* FILE, fprintf */
#endif    /* !NDEBUG */

#include "common.h"
#include "in.h"
#include "strg.h"
#include "lmap.h"

#define LPUNIT 1024    /* allocation unit for line positions */
#define LBUNIT 256     /* allocation unit for line buffer */


/* internal functions referenced forwardly */
static const lmap_t *generate(int, sz_t);


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
static lmap_t dummy;                     /* dummy locus used by fixed() */


const lmap_t *lmap_from;                            /* current from node */
const lmap_t *lmap_cmd;                             /* command line locus */
const lmap_t *lmap_bltin;                           /* built-in locus */
const lmap_t *(*lmap_add)(int, sz_t) = generate;    /* function to get source locus */


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

    if (!*rf || rf == lmap_bltin->from->u.i.rf || rf == lmap_cmd->from->u.i.rf)
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
 *  (source locus) generates a source locus
 */
static const lmap_t *generate(int dy, sz_t wx)
{
    lmap_t *p = ARENA_ALLOC(strg_perm, sizeof(*p));

    assert(lmap_from->type != LMAP_MACRO);

    p->type = LMAP_NORMAL;
    p->u.n.py = in_py + dy;
    p->u.n.wx = wx;
    p->u.n.dy = 0;
    p->u.n.dx = wx + 1;
    p->from = lmap_from;

    return p;
}


/*
 *  (source locus) returns a dummy locus
 */
static const lmap_t *fixed(int dy, sz_t wx)
{
    UNUSED(dy);
    UNUSED(wx);

    return &dummy;    /* let lex_next() modify */
}


/*
 *  (source locus) makes lex_next() use a generated or dummy locus
 */
void (lmap_setadd)(int clear)
{
    lmap_add = (clear)? generate: fixed;
}


/*
 *  (source locus) constructs a range from two source loci
 */
const lmap_t *(lmap_range)(const lmap_t *s, const lmap_t *e)
{
    lmap_t *p;

    assert(s);

    if (!e || s == e)
        return s;

    s = lmap_mstrip(s);
    e = lmap_mstrip(e);

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
 *  (source locus) constructs a locus from a pointer into a clean spelling
 */
const lmap_t *(lmap_spell)(const lmap_t *pos, const char *p, const char *s, const char *rs,
                           const char *re)
{
    int dy;
    lmap_t *npos;

    assert(pos);
    assert(p);
    assert(s);
    assert(rs);
    assert(re);

    if (pos->type != LMAP_NORMAL)
        return pos;

    npos = ARENA_ALLOC(strg_line, sizeof(*npos));
    memcpy(npos, pos, sizeof(*npos));

    if (p == s)    /* clean */
        s = rs;    /* retains re */
    else {
        in_cntchar(p, NULL, in_cntchar(s, rs, (sz_t)-1, NULL), &s);
        in_cntchar(s, NULL, in_cntchar(rs, re, (sz_t)-1, NULL), &re);
    }
    npos->u.n.wx = in_getwx(npos->u.n.wx, p, s, &dy);
    npos->u.n.py += dy;
    npos->u.n.dx = in_getwx(npos->u.n.wx, s, re, &dy);
    npos->u.n.dy = dy;

    return npos;
}


/*
 *  (source locus) constructs a locus to indicate #include
 */
const lmap_t *(lmap_include)(const char *rf, const char *f, const lmap_t *from, int sys)
{
    lmap_t *p;

    assert(rf);
    assert(f);
    assert(from);
    assert(from->type == LMAP_NORMAL);

    p = ARENA_ALLOC(strg_perm, sizeof(*p));
    p->type = LMAP_INC;
    p->u.i.f = f;
    p->u.i.yoff = 0;    /* for cis */
    p->u.i.rf = rf;
    p->u.i.printed = 0;
    p->u.i.system = sys;
    p->from = from;

    return p;
}


/*
 *  (source locus) constructs a locus to indicate #line
 */
const lmap_t *(lmap_line)(const char *s, sz_t yoff, const lmap_t *f)
{
    lmap_t *p;

    assert(s);
    assert(f);

    p = ARENA_ALLOC(strg_perm, sizeof(*p));
    p->type = LMAP_LINE;
    p->u.l.f = s;
    p->u.l.yoff = yoff;
    p->from = f;

    return p;
}


/*
 *  (source locus) constructs a locus to indicate macro expansion
 */
const lmap_t *(lmap_macro)(const lmap_t *o, const lmap_t *f, arena_t *a)
{
    lmap_t *p;

    assert(o);
    assert(f);
    assert(a);

    p = ARENA_ALLOC(a, sizeof(*p));
    p->type = LMAP_MACRO;
    p->u.m = o;
    p->from = f;

    return p;
}


/*
 *  (source locus) finds a from node for nominal or physical information
 */
const lmap_t *(lmap_npfrom)(int n, const lmap_t *p)
{
    assert(p);

    while (p->type >= LMAP_LINE+n)
        p = p->from;

    assert(p);
    return p;
}


/*
 *  (source locus) strips off LMAP_MACRO nodes
 */
const lmap_t *(lmap_mstrip)(const lmap_t *p)
{
    assert(p);

    while (p->type == LMAP_MACRO)
        p = p->from;

    assert(p->type == LMAP_NORMAL);
    return p;
}


/*
 *  initializes the line mapper
 */
void (lmap_init)(const char *rf, const char *f)
{
    static lmap_t root;
    static lmap_t cmdh = { -1, { "<command-line>", 0, "<command-line>", 0, 0 }, NULL },
                  blth = { -1, { "<built-in>",     0, "<built-in>",     0, 0 }, NULL },
                  cmdn = { LMAP_NORMAL, { NULL, }, &cmdh },
                  bltn = { LMAP_NORMAL, { NULL, }, &blth };

    root.type = -1;
    root.u.i.f = f;
    root.u.i.rf = rf;
    lmap_from = &root;

    bltn.u.n.py = cmdn.u.n.py = 1;
    bltn.u.n.wx = cmdn.u.n.wx = 1;
    bltn.u.n.dy = cmdn.u.n.dy = 0;
    bltn.u.n.dx = cmdn.u.n.dx = 2;
    lmap_cmd = &cmdn;
    lmap_bltin = &bltn;
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


#ifndef NDEBUG
/*
 *  prints a locus chain for debugging
 */
void (lmap_print)(const lmap_t *pos, FILE *fp)
{
    assert(fp);

    if (!pos)
        pos = lmap_from;
    assert(pos);

    do {
        switch(pos->type) {
            case -1:    /* root */
                fprintf(fp, "[%p] root: %s %s\n", (void *)pos, pos->u.i.f, pos->u.i.rf);
                break;
            case LMAP_INC:
                fprintf(fp, "[%p] #include: %s %s%s%s\n", (void *)pos, pos->u.i.f, pos->u.i.rf,
                        (pos->u.i.printed)? " p": "", (pos->u.i.system)? " s": "");
                break;
            case LMAP_LINE:
                fprintf(fp, "[%p] #line: %s %"FMTSZ"u\n", (void *)pos, pos->u.l.f, pos->u.l.yoff);
                break;
            case LMAP_MACRO:
                fprintf(fp, "[%p] macro: %p\n", (void *)pos, (void *)pos->u.m);
                break;
            case LMAP_NORMAL:
                fprintf(fp, "[%p] normal: %"FMTSZ"u %"FMTSZ"u %d %"FMTSZ"u\n", (void *)pos,
                        pos->u.n.py, pos->u.n.wx, pos->u.n.dy, pos->u.n.dx);
                break;
        }
        pos = pos->from;
    } while(pos);
}
#endif    /* !NDEBUG */

/* end of lmap.c */
