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
 *  drives preprocessing
 */
void (cpp_start)(FILE *fp)
{
    sz_t ty;
    const char *tf;
    int ptid;
    int needsp, sync;
    lex_t *t, *n;
    const lmap_t *npos, *fpos;

    assert(fp);

    ty = 1;
    tf = lmap_from->u.i.f;
    ptid = 0;
    needsp = sync = 0;
    outfile = fp;

    proc_prep();
    n = lst_peek();
    if (n->id != LEX_EOI) {
        npos = lmap_mstrip(n->pos);
        fpos = lmap_nfrom(npos);
        if (npos->u.n.py+fpos->u.i.yoff != ty || fpos->type != -1) {
            sync = 1;
            ty = npos->u.n.py+fpos->u.i.yoff;
            tf = fpos->u.i.f;
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
                if (!sync) {
                    putc('\n', outfile);
                    npos = t->pos->from;
                    assert(npos->type <= LMAP_LINE);
                    if (ty != t->pos->u.n.py+npos->u.i.yoff || tf != npos->u.i.f)
                        sync = 1;
                    else
                        ty++;
                }
                n = lst_peek();
                if (n->id == LEX_EOI)
                    continue;
                npos = lmap_mstrip(n->pos);
                fpos = lmap_nfrom(npos);
                if (sync || ty != npos->u.n.py+fpos->u.i.yoff || tf != fpos->u.i.f) {
                    sync = 1;
                    ty = npos->u.n.py+fpos->u.i.yoff;
                    tf = fpos->u.i.f;
                }
                ptid = t->id;
                break;
            default:
                if (sync) {
                    fprintf(outfile, "# %"FMTSZ"u \"", ty);
                    printesc(tf);
                    fputs("\"\n", outfile);
                    sync = 0;
                }
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
}

/* end of cpp.c */
