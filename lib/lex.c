/*
 *  lexical analyzer for preprocessor
 */

#include <stddef.h>        /* size_t */
#include <cbl/assert.h>    /* assert */
#include <cbl/memory.h>    /* MEM_ALLOC, MEM_CALLOC, MEM_RESIZE */

#include "common.h"
#include "err.h"
#include "in.h"
#include "util.h"
#include "lex.h"

/* puts character into token buffer */
#define putbuf(c) (((pbuf == buf+bsize-1)? resize(): (void)0), *pbuf++ = (c))

/* allocates buffer for token spelling */
#define NEWBUF() (bsize=IN_MAXTOKEN, pbuf=buf=MEM_CALLOC(1, bsize))

/* handles line splicing */
#define BSNL()                                          \
    do {                                                \
        do { in_nextline(); } while(*in_cp == '\n');    \
        wx = 1, rcp = in_cp;                            \
    } while(0)

/* returns a token consisted of a single-byte character */
#define RETSB(c, i, r)                \
    do {                              \
        wx += (c);                    \
        in_cp += (c);                 \
        ptok->id = (i);               \
        ptok->spell = (char *)(r);    \
        return ptok;                  \
    } while(0)

/* returns a token consisted of two and more single-byte characters */
#define RETSBN(c, i, r)               \
    do {                              \
        wx += (c);                    \
        in_cp += (c);                 \
        ptok->id = (i);               \
        ptok->spell = (char *)(r);    \
        ptok->pos->u.n.n = 1+(c);     \
        return ptok;                  \
    } while(0)

/* returns EOI or NEWLINE, and resets wx */
#define RETNL(i)             \
    do {                     \
        wx = 1;              \
        ptok->id = (i);      \
        ptok->spell = "";    \
        return ptok;         \
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
 *  recognizes a [line splicing| ]+[trigraph|character]
 */
static int bsnl3(int *pn)
{
    int c;
    int n = 1;

    if (*in_cp == '\n') {
        do { in_nextline(); } while(*in_cp == '\n');
        wx = 1;
        pn = NULL;
    }
    if (main_opt()->trigraph && in_cp[0] == '?' && in_cp[1] == '?') {
        c = conv3(in_cp[2]);
        if (c != '?') {
            in_trigraph(in_cp);
            if ((main_opt()->trigraph & 1) != 0)
                n = 3;
            else
                c = '?';
        }
    } else
        c = *in_cp;

    if (pn)
        *pn = n;
    return c;
}


/*
 *  retrieves a token from the input stream
 */
lex_t *(lex_nexttok)(void)
{
    int c;
    int n = 0;
    lex_t *ptok;

    assert(in_cp);
    assert(in_limit);

    ptok = MEM_ALLOC(sizeof(*ptok));
    ptok->pos = lmap_add(wx, 1);    /* n set later if necessary */

    while (1) {
        register const char *rcp = in_cp;

        in_cp = rcp + 1, wx++;
        switch(*rcp++) {
            /* whitespaces */
            case '\n':    /* line splicing */
                in_nextline();
                wx = 1;
                rcp = in_cp;
                break;
            case '\0':
                /* EOI is detected with unusual check
                   because newline constitutes valid token */
                if (in_cp > in_limit) {
                    in_cp = in_limit;
                    RETNL(LEX_EOI);
                } else {
                    assert(!fromstr);
                    in_nextline();
                    RETNL(LEX_NEWLINE);
                }
            case '\v':
            case '\f':
            case '\r':
            case ' ':
            case '\t':
                NEWBUF();
                rcp--;
                while (1) {
                    while (*rcp == ' ' || *rcp == '\t' || *rcp == '\v' || *rcp == '\f' ||
                           *rcp == '\r')
                        putbuf(*rcp++);
                    if (ptok->pos->u.n.py == in_py)
                        ptok->pos->u.n.n = 1+rcp-in_cp;
                    if (*rcp != '\n')
                        break;
                    BSNL();
                }
                RETSB(rcp - in_cp, LEX_SPACE, buf);
            /* punctuations */
            case '!':    /* != !  !\[= ] */
                if (*rcp == '=')
                    RETSBN(1, LEX_NEQ, "!=");
                if (*rcp != '\n')
                    RETSB(0, '!', "!");
                BSNL();
                if (*rcp == '=')
                    RETSBN(0, LEX_NEQ, "!=");
                RETSB(0, '!', "!");
            case '#':    /* ## #  #[\ ][??= # ] */
                if (*rcp == '#')
                    RETSBN(1, LEX_DSHARP, "##");
                if (*rcp != '\n' && *rcp != '?')
                    RETSB(0, LEX_SHARP, "#");
                c = bsnl3(&n);
                if (c == '#')
                    RETSBN(n, LEX_DSHARP, "##");
                RETSB(0, LEX_SHARP, "#");
        }
    }
}

/* end of lex.c */
