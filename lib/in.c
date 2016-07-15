/*
 *  input handling
 */

#include <stddef.h>        /* NULL */
#include <stdio.h>         /* FILE, fgets, ferror, feof */
#include <string.h>        /* strlen */
#include <cbl/assert.h>    /* assert */
#include <cbl/memory.h>    /* MEM_ALLOC, MEM_RESIZE, MEM_FREE */
#include <cdsl/hash.h>     /* hash_string */

#include "common.h"
#include "err.h"
#include "lmap.h"
#include "main.h"
#include "util.h"
#include "in.h"


sz_t in_py;                   /* physical line # of current file */
const char *in_line;          /* beginning of current line */
const char *in_cp;            /* current character */
const char *in_limit;         /* end of current line */
void (*in_nextline)(void);    /* function to read next input line */


static FILE *fptr;    /* file pointer for input */
static char *buf;     /* input buffer */
static sz_t bufn;     /* input buffer size */
#ifdef HAVE_ICONV
static char *ibuf;    /* UTF-8 input buffer */
static sz_t ibufn;    /* UTF-8 input buffer size */
#endif    /* HAVE_ICONV */


/*
 *  counts the number of characters in UTF-8 strings;
 *  ASSUMPTION: (HAVE_ICONV) UTF-8 used as internal pivot encoding
 */
sz_t (in_cntchar)(const char *p, const char *q, sz_t m, const char **pp)
{
    sz_t n = 0;

    while ((!q || p < q) && (m == (sz_t)-1 || n < m)) {
#if HAVE_ICONV
        if (!main_iton || FIRSTUTF8(*p))
#endif    /* HAVE_ICONV */
            n++;
        if (main_opt()->trigraph && p[0] == '?' && p[1] == '?') {
            switch(p[2]) {
                case '(':    /* [ */
                case ')':    /* ] */
                case '<':    /* { */
                case '>':    /* } */
                case '=':    /* # */
                case '/':    /* \ */
                case '\'':    /* ^ */
                case '!':    /* | */
                case '-':    /* ~ */
                    p += 2;
                    break;
            }
        }
        p++;
    }
#if HAVE_ICONV
    if (main_iton)
        while (!FIRSTUTF8(*p) && p < q)
            p++;
#endif    /* HAVE_ICONV */

    if (pp)
        *pp = p;    /* q or points to char after m chars */
    return n;
}


/*
 *  replaces nextline() after EOF seen
 */
static void eof(void)
{
    in_limit = in_cp;
    return;
}


/*
 *  reads the next line;
 *  in_limit points to one past the terminating null unless EOF;
 *  ASSUMPTION: '\n' is no part of multibyte characters and has no effect on the shift state;
 *  ASSUMPTION: charset in which source is written is same as that in which beluga is running
 */
static void nextline(void)
{
    static int state;
    static sz_t alen;

    char *p;
    sz_t len;
    int bs = 0;

    assert(fptr);

    lmap_fline(in_py+1, ftell(fptr));
    p = (char *)(in_limit = in_line = in_cp = buf);
    *p = '\0';
    len = 0;

    while (1) {
        assert(bufn-len > 1);
        /* retrun value of fgets() need not be checked;
           *p is NUL and len is 0 for start of each line */
        fgets((char *)p+len, bufn-len, fptr);
        if (ferror(fptr)) {
            err_issuel(NULL, 1, ERR_INPUT_ERROR);
            in_nextline = eof;
            break;
        }
        len += strlen((char *)p+len);
        if (len == 0) {    /* real EOF */
            in_py++;
            in_nextline = eof;
            break;
        }
        if (p[len-1] == '\n' || feof(fptr)) {    /* line completed */
            in_py++;
#ifdef HAVE_ICONV
            if (main_iton) {
                ICONV_DECL((char *)p, len + 1);    /* +1 to include NUL */
                olenv = olen = ibufn;
                obufv = obuf = ibuf;
                ICONV_DO(main_iton, 0, {});
                ibuf = p = obuf;
                len = olen - olenv - 1;
            }
#endif    /* HAVE_ICONV */
            in_line = p;
            if (p[len-1] == '\n') {
                if ((len > 1 && p[len-2] == '\\') ||
                    (main_opt()->trigraph && len > 3 &&
                     p[len-4] == '?' && p[len-3] == '?' && p[len-2] == '/')) {
                    int c = getc(fptr), n = 1;
                    if (c == EOF) {
                        if (p[len-2] == '/')
                            len -= 2, n = 3;
                        err_issuel(p+len-2, n, ERR_INPUT_BSNLEOF);
                    } else
                        ungetc(c, fptr);
                    p[len-2] = '\n';
                    if (main_opt()->std && state < 2) {
                        const char *q;
                        sz_t c = in_cntchar(p, &p[len-2], TL_LINE_STD-alen, &q);
                        bs = state = 1;
                        if ((alen += c) >= TL_LINE_STD) {
                            state = 2;
                            err_issuel(q, 1, ERR_INPUT_LONGLINE);
                            err_issuel(NULL, 1, ERR_INPUT_LONGLINESTD, (unsigned long)TL_LINE_STD);
                        }
                    }
                }
                p[--len] = '\0';
            } else
                err_issuel(p+len, 1, ERR_INPUT_NOTENDNL);
            in_limit = &p[len+1];
            in_cp = p;
            if (main_opt()->std && !bs) {
                if (state < 2) {
                    const char *q;
                    sz_t c = in_cntchar(p, &p[len], TL_LINE_STD-alen, &q);
                    if (alen + c >= TL_LINE_STD) {
                        err_issuel(q, 1, ERR_INPUT_LONGLINE);
                        err_issuel(NULL, 1, ERR_INPUT_LONGLINESTD, (unsigned long)TL_LINE_STD);
                    }
                }
                alen = 0, state = 0;
            }
            return;
        } else {    /* expands buffer */
            MEM_RESIZE(p, bufn+=BUFUNIT);
            buf = p;
        }
    }
    /* EOF */
    p[0] = '\0';
    in_limit = in_cp = in_line;
}


/*
 *  prepares to accept input;
 *  also called when an input file changed
 */
void (in_init)(FILE *fp, const char *fn)
{
    assert(fp);

    fptr = fp;
    if (!fn)
        fn = "";

    fn = hash_string(fn);
    if (*fn) {
        lmap_init(fn, rpath(fn));
        lmap_flset(fn);
    }

    assert(BUFUNIT > 1);
    buf = MEM_ALLOC(bufn = BUFUNIT);
#ifdef HAVE_ICONV
    if (main_iton)
        ibuf = MEM_ALLOC(ibufn = BUFUNIT);
#endif    /* HAVE_ICONV */
    in_nextline = nextline;
    nextline();
}


/*
 *  finalizes input
 */
void (in_close)(void)
{
    MEM_FREE(buf);
#ifdef HAVE_ICONV
    if (main_iton)
        MEM_FREE(ibuf);
#endif    /* HAVE_ICONV */
    in_limit = in_line = in_cp = NULL;
}


/*
 *  counts characters with wcwidth();
 *  ASSUMPTION: the result is less than max of sz_t
 */
sz_t (in_getwx)(const char *s, const char *p)
{
    sz_t wx = 1;

    assert(s);
    assert(p);

    while (s < p) {
        unsigned long wc = utf8to32(&s);
        if (wc == -1)
            return -1;
        wx += wcwidth(wc);
    }

    return wx;
}

/* end of in.c */
