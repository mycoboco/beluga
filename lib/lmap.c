/*
 *  line mapper
 */

#include <stddef.h>        /* NULL */
#include <stdio.h>         /* FILE, FOPEN_MAX, getc, EOF, ferror, feof, fopen, fclose, fseek,
                              rewind, sprintf */
#include <string.h>        /* strlen, strcpy, memcpy */
#include <cbl/memory.h>    /* MEM_ALLOC, MEM_RESIZE, MEM_FREE */
#include <cbl/arena.h>     /* arena_t, ARENA_ALLOC */
#include <cbl/assert.h>    /* assert */
#ifndef NDEBUG
#include <stdio.h>         /* fprintf */
#endif    /* !NDEBUG */

#include "common.h"
#include "in.h"
#include "lex.h"
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

static char buf[LBUNIT], *pbuf = buf;     /* line buffer */
static sz_t bufn = NELEM(buf);            /* size of line buffer */
static const char *nstdin = "<stdin>";    /* name for stdin */
static lmap_t dummy;                      /* dummy locus used by fixed() */


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
 *  replaces fgets() to handle embedded null characters
 */
static char *ngets(char *s, int n, FILE *fp)
{
    int c;
    char *p = s;

    assert(s);
    assert(fp);

    while (--n > 0) {
        if ((c = getc(fp)) == EOF)
            break;
        if (c == '\0')
            c = ' ';
        *p++ = c;
        if (c == '\n')
            break;
    }
    *p = 0;

    return p;
}


/*
 *  (line location) reads a line from a file
 */
static const char *line(FILE *fp)
{
    sz_t len = 0;
    const char *q;

    pbuf[0] = '\n', pbuf[1] = '\0';    /* for one past last line */

    while (1) {
        q = ngets(pbuf+len, bufn-len, fp);
        if (ferror(fp)) {
            rewind(fp);
            return NULL;
        }
        len += (q - (pbuf+len));
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

    if (rf == nstdin || rf == lmap_bltin->from->u.i.rf || rf == lmap_cmd->from->u.i.rf)
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
 *  checks if two loci are on the same physical line
 */
static int sameline(const lmap_t *p, const lmap_t *q)
{
    assert(p->type == LMAP_NORMAL);
    assert(q->type == LMAP_NORMAL);

    if (p->u.n.py == q->u.n.py)
        return (lmap_pfrom(p)->u.i.rf == lmap_pfrom(q)->u.i.rf);
    return 0;
}


/*
 *  checks if two tokens came from the same invocation sequence
 */
static int sameseq(const lmap_t *p, const lmap_t *q)
{
    assert(p);
    assert(p->type == LMAP_MACRO);
    assert(q);
    assert(q->type == LMAP_MACRO);

    p = p->from;
    q = q->from;
    while (p->type == LMAP_MACRO && p == q)
        p = p->from, q = q->from;

    return (p->type == LMAP_NORMAL && p == q);
}


/*
 *  (source locus) constructs a range from two source loci
 */
const lmap_t *(lmap_range)(const lmap_t *s, const lmap_t *e)
{
    lmap_t *p;
    const lmap_t *m;

    assert(s);

    if (!e || s == e)
        return s;

    m = NULL;
    if (s->type == LMAP_MACRO && e->type == LMAP_MACRO) {    /* both from macro */
        const lmap_t *ss, *se;
        for (ss = s; ss->type == LMAP_MACRO; ss = ss->from)
            for (se = e; se->type == LMAP_MACRO; se = se->from)
                if (sameseq(ss, se)) {
                    if (ss->type == LMAP_MACRO)
                        ss = ss->u.m;
                    if (se->type == LMAP_MACRO)
                        se = se->u.m;
                    if (ss != se && sameline(ss, se))
                        m = s->from, s = ss, e = se;
                }
    }
    s = lmap_mstrip(s);
    e = lmap_mstrip(e);
    if (s == e)
        return s;

    p = ARENA_ALLOC(strg_perm, sizeof(*p));
    p->type = LMAP_NORMAL;
    p->u.n.py = s->u.n.py;
    p->u.n.wx = s->u.n.wx;
    p->u.n.dy = e->u.n.py+e->u.n.dy - s->u.n.py;
    if (p->u.n.dy < 0)    /* s and e might be reversed */
        p->u.n.dy = 0;
    p->u.n.dx = e->u.n.dx;
    if (p->u.n.dx <= p->u.n.wx)
        p->u.n.dx = p->u.n.wx + 1;
    p->from = s->from;

    return (m)? lmap_macro(p, m, strg_perm): p;
}


/*
 *  (source locus) constructs a locus from a pointer into a clean spelling
 */
const lmap_t *(lmap_spell)(lex_t *t, const char *s, const char *rs, const char *re)
{
    int dy;
    lmap_t *n;

    assert(t);
    assert(s);
    assert(rs);
    assert(re);

    if (t->pos->type != LMAP_NORMAL)
        return t->pos;

    n = ARENA_ALLOC(strg_line, sizeof(*n));
    memcpy(n, t->pos, sizeof(*n));

    if (t->spell == s)    /* clean */
        s = rs;    /* retains re */
    else {
        in_cntchar(t->spell, NULL, in_cntchar(s, rs, (sz_t)-1, NULL), &s);
        while (*s == '\n')
            s++;
        in_cntchar(s, NULL, in_cntchar(rs, re, (sz_t)-1, NULL), &re);
    }
    n->u.n.wx = in_getwx(n->u.n.wx, t->spell, s, &dy);
    n->u.n.py += dy;
    n->u.n.dx = in_getwx(n->u.n.wx, s, re, &dy);
    if (n->u.n.dx == n->u.n.wx)    /* *rs might be NUL */
        n->u.n.dx++;
    n->u.n.dy = dy;

    return n;
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

    if (o->type == LMAP_MACRO)
        o = o->u.m;
    assert(o->type == LMAP_NORMAL);

    p = ARENA_ALLOC(a, sizeof(*p));
    p->type = LMAP_MACRO;
    p->u.m = o;
    p->from = f;

    return p;
}


/*
 *  (source locus) constructs a locus after a token;
 *  must be immediately consumed for diagnostics
 */
const lmap_t *(lmap_after)(const lmap_t *p)
{
    assert(p);
    assert(p->type == LMAP_MACRO || p->type == LMAP_NORMAL);

    dummy.type = LMAP_AFTER;
    dummy.from = p;
    /* other fields left dirty */

    return &dummy;
}


/*
 *  (source locus) constructs a locus at the start of a token;
 *  must be immediately consumed for diagnostics
 */
const lmap_t *(lmap_pin)(const lmap_t *p)
{
    assert(p);
    assert(p->type == LMAP_MACRO || p->type == LMAP_NORMAL);

    dummy.type = LMAP_PIN;
    dummy.from = p;
    /* other fields left dirty */

    return &dummy;
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
 *  (source locus) converts a locus to a string;
 *  should not be invoked more than once in the same call;
 *  see also ty_outtype()
 */
const char *(lmap_out)(const lmap_t *p)
{
    static char buf[80];

    sz_t len;
    char *pbuf = buf;
    const lmap_t *f;

    assert(p);

    p = lmap_mstrip(p);
    assert(p->type == LMAP_NORMAL);
    f = lmap_nfrom(p);
    len = strlen(f->u.i.f);    /* cis */
    if (sizeof(buf) < len)
        pbuf = ARENA_ALLOC(strg_line, len);
    sprintf(pbuf, "%s:%"FMTSZ"u:%"FMTSZ"u", f->u.i.f, p->u.n.py+f->u.i.yoff,    /* cis */
                  p->u.n.wx);

    return pbuf;
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
    root.u.i.f = (f)? f: nstdin;
    root.u.i.rf = (rf)? rf: nstdin;
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
            case LMAP_AFTER:
                fprintf(fp, "[%p] after\n", (void *)pos);
                break;
            case LMAP_PIN:
                fprintf(fp, "[%p] pin\n", (void *)pos);
                break;
            default:
                assert(!"invalid node type -- should never reach here");
                break;
        }
        pos = pos->from;
    } while(pos);
}
#endif    /* !NDEBUG */

/* end of lmap.c */
