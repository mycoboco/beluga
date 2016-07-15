/*
 *  lexical analyzer for preprocessor
 */

#include <stddef.h>        /* size_t */
#include <cbl/assert.h>    /* assert */
#include <cbl/memory.h>    /* MEM_ALLOC, MEM_CALLOC, MEM_RESIZE */

#include "common.h"
#include "err.h"
#include "in.h"
#include "lex.h"

/* puts character into token buffer */
#define putbuf(c) (((pbuf == buf+bsize-1)? resize(): (void)0), *pbuf++ = (c))

/* allocates buffer for token spelling */
#define NEWBUF() (bsize=IN_MAXTOKEN, pbuf=buf=MEM_CALLOC(1, bsize))

/* returns a token */
#define RETURN(c, i, r)                     \
    do {                                    \
        ptok->pos = lmap_add(wx, 1+(c));    \
        wx += 1+(c);                        \
        in_cp += (c);                       \
        ptok->id = (i);                     \
        ptok->spell = (char *)(r);          \
        return ptok;                        \
    } while(0)

/* returns a token and resets wx */
#define RETURNNL(i)                     \
    do {                                \
        ptok->pos = lmap_add(wx, 1);    \
        wx = 0;                         \
        ptok->id = (i);                 \
        ptok->spell = "";               \
        return ptok;                    \
    } while(0)


static sz_t wx = 1;         /* x counted by wcwidth() */
static int fromstr;         /* true while input coming from string */
static sz_t bsize;          /* size of token buffer */
static char *buf, *pbuf;    /* pointers to maintain token buffer */


/*
 *  enlarges a token buffer
 */
static void resize(void)
{
    MEM_RESIZE(buf, bsize+IN_MAXTOKEN);
    pbuf = buf + bsize - 1;
    bsize += IN_MAXTOKEN;
}


/*
 *  retrieves a token from the input stream
 */
lex_t *(lex_nexttok)(void)
{
    lex_t *ptok;

    assert(in_cp);
    assert(in_limit);

    ptok = MEM_ALLOC(sizeof(*ptok));

    while (1) {
        register const char *rcp = in_cp;

        in_cp = rcp + 1;
        switch(*rcp++) {
            /* whitespaces */
            case '\n':    /* line splicing */
                break;
            case '\0':
                /* EOI is detected with unusual check
                   because newline constitutes valid token */
                if (in_cp > in_limit) {
                    in_cp = in_limit;
                    RETURNNL(LEX_EOI);
                } else {
                    assert(!fromstr);
                    in_nextline();
                    RETURNNL(LEX_NEWLINE);
                }
            case '\v':
            case '\f':
            case '\r':
            case ' ':
            case '\t':
                NEWBUF();
                rcp--;
                do {
                    putbuf(*rcp++);
                } while (*rcp == ' ' || *rcp == '\t' || *rcp == '\v' || *rcp == '\f' ||
                         *rcp == '\r');
                RETURN(rcp - in_cp, LEX_SPACE, buf);
        }
    }
}

/* end of lex.c */
