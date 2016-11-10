/*
 *  preprocessor
 */

#include <stdio.h>         /* fputs, putc, fprintf */
#include <cbl/assert.h>    /* assert */

#include "lex.h"
#include "lst.h"
#include "proc.h"
#include "strg.h"
#include "cpp.h"


/* 0 if no spaces are necessary;
   1 if space is necessary after the token;
   2 if space is necessary before the token and
   3 if spaces are necessary before and after the token */
static char toksp[] = {
#define xx(a, b, c, d, e, f, g, h) h,
#define yy(a, b, c, d, e, f, g, h) h,
#include "xtoken.h"
};

static FILE *outfile;    /* output file */
static int sync;         /* true if line sync is necessary */


/*
 *  prints a string escaping special characters
 */
static void printesc(const char *s)
{
    assert(s);

    for (; *s; s++)
        switch(*s) {
            case '\\':
                fputs("\\\\", outfile);
                break;
            case '"':
                fputs("\\\"", outfile);
                break;
            /* others are printed without escaping */
            case '\a':
            case '\b':
            case '\f':
            case '\n':
            case '\r':
            case '\t':
            default:
                putc(*s, outfile);
                break;
        }
}


/*
 *  prints gcc-style line synchronization
 */
static void outpos(const char *f, sz_t y, int n)
{
    static const char *ns[] = { "", " 1", " 2" };

    assert(f);
    assert(n >= 0 && n < NELEM(ns));

    fprintf(outfile, "# %"FMTSZ"u \"", y);
    printesc(f);
    fprintf(outfile, "\"%s\n", ns[n]);
    sync = 0;
}


/*
 *  drives preprocessing
 */
void (cpp_start)(FILE *fp)
{
    sz_t ty;
    const char *tf;
    int ptid;
    int needsp;
    lex_t *t, *n;
    const lmap_t *npos, *fpos;

    assert(fp);

    ty = 1;
    tf = lmap_from->u.i.f;
    ptid = 0;
    needsp = 0;
    outfile = fp;

    outpos(tf, ty, 0);
    proc_prep();
    n = lst_peek();
    if (n->id != LEX_EOI) {
        npos = lmap_mstrip(n->pos);
        fpos = lmap_nfrom(npos);
        if (npos->u.n.py+fpos->u.i.yoff != ty || fpos->type != -1) {
            sync = 1;
            tf = fpos->u.i.f;
            ty = npos->u.n.py+fpos->u.i.yoff;
        }
    }
    while ((t = lst_next())->id != LEX_EOI) {
        switch(t->id) {
            case -1:
                strg_free((arena_t *)t->spell);
                break;
            case LEX_MCR:
                needsp = toksp[ptid] & 1;
                break;
            case LEX_NEWLINE:
                if (t->f.sync) {    /* met #include boundaries */
                    npos = t->pos->from;
                    assert(npos->type <= LMAP_LINE);
                    if (sync || ty != t->pos->u.n.py+npos->u.i.yoff || tf != npos->u.i.f)
                        outpos(npos->u.i.f, t->pos->u.n.py+npos->u.i.yoff, sync >> 1);
                    n = lst_peek();
                    npos = lmap_mstrip(n->pos);
                    fpos = lmap_nfrom(npos);
                    sync |= (1 << t->f.sync);
                    tf = fpos->u.i.f;
                    ty = npos->u.n.py+fpos->u.i.yoff;
                } else {
                    if (!sync) {
                        putc('\n', outfile);
                        ty++;
                    }
                    n = lst_peek();
                    npos = lmap_mstrip(n->pos);
                    fpos = lmap_nfrom(npos);
                    if (sync || ty != npos->u.n.py+fpos->u.i.yoff || tf != fpos->u.i.f) {
                        sync |= 1;
                        tf = fpos->u.i.f;
                        ty = npos->u.n.py+fpos->u.i.yoff;
                    }
                }
                ptid = t->id;
                break;
            default:
                if (sync)
                    outpos(tf, ty, sync >> 1);
                if (needsp) {
                    if (toksp[t->id] & 2)
                        putc(' ', outfile);
                    needsp = 0;
                }
                fputs(LEX_SPELL(t), outfile);
                ptid = t->id;
                break;
        }
    }
    if (sync > 1)    /* cares #include sync only */
        outpos(tf, ty, sync >> 1);
}

/* end of cpp.c */
