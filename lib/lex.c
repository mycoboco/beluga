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
#define BSNL()                                 \
    do {                                       \
        do { dy++; } while(*++rcp == '\n');    \
        wx = 1;                                \
        in_cp = rcp;                           \
    } while(0)

/* returns a token */
#define RETURN(x, l, i, s)            \
    do {                              \
        wx += (x);                    \
        in_cp += (x);                 \
        ptok->id = (i);               \
        ptok->spell = (char *)(s);    \
        ptok->pos->u.n.n += (l);      \
        return ptok;                  \
    } while(0)

/* returns a token after line splicing and/or trigraphs */
#define RETSET(d, i, s)                 \
    do {                                \
        dy += pr[d].dy;                 \
        wx = pr[d].wx;                  \
        in_cp = pr[d].p;                \
        ptok->id = (i);                 \
        ptok->spell = (char *)(s);      \
        ptok->pos->u.n.n += pr[d].n;    \
        return ptok;                    \
    } while(0)


/* result type for bsnl3() */
struct bs_t {
    int dy;           /* adjusts dy */
    int wx;           /* sets wx */
    int n;            /* adjusts token length */
    const char *p;    /* sets in_cp */
    char c;           /* character read */
};


static int dy = 0;          /* adjusts py for line splicing */
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
 *  recognizes instances of [line splicing| ]+[trigraph|character]
 */
static struct bs_t *bsnl3(void)
{
    static struct bs_t r[3+1];    /* reads at most 3 chars */

    int i, n = 1;
    register const char *rcp = in_cp;

    /* r[0].dy = 0; */
    r[0].wx = wx;

    for (i = 1; i < NELEM(r); i++, rcp++) {
        r[i].dy = r[i-1].dy;
        r[i].wx = r[i-1].wx;

        if (*rcp == '\n') {
            do { r[i].dy++; } while(*++rcp == '\n');
            r[i].wx = 1;
            n = 0;
        }
        if (main_opt()->trigraph && rcp[0] == '?' && rcp[1] == '?') {
            r[i].c = conv3(rcp[2]);
            if (r[i].c != '?') {
                in_trigraph(rcp);
                if (main_opt()->trigraph & 1) {
                    rcp += 2;
                    r[i].wx += 2;
                    if (n > 0)
                        n = 3;
                } else
                    r[i].c = '?';
            }
        } else
            r[i].c = *rcp;
        r[i].wx++;
        r[i].p = rcp+1;

        r[i].n = (n > 0)? n: r[i-1].n;
        if (*rcp == '\0')
            break;
    }

    return &r[1];
}


/*
 *  retrieves a token from the input stream
 */
lex_t *(lex_nexttok)(void)
{
    lex_t *ptok;
    struct bs_t *pr;

    assert(in_cp);
    assert(in_limit);

    ptok = MEM_ALLOC(sizeof(*ptok));
    ptok->pos = lmap_add(dy, wx, 1);    /* n adjusted later if necessary */

    while (1) {
        register const char *rcp = in_cp;

        in_cp = rcp + 1, wx++;
        switch(*rcp++) {
            /* whitespaces */
            case '\n':    /* line splicing */
                dy++;
                ptok->pos->u.n.py++;
                ptok->pos->u.n.wx = wx = 1;
                break;
            case '\0':    /* line end */
                /* EOI is detected with unusual check
                   because newline constitutes valid token */
                dy = 0, wx = 1;
                if (in_cp > in_limit) {
                    in_cp = in_limit;
                    RETURN(0, 0, LEX_EOI, "");
                } else {
                    assert(!fromstr);
                    in_nextline();
                    RETURN(0, 0, LEX_NEWLINE, "");
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
                    if (ptok->pos->u.n.py == in_py + dy)
                        ptok->pos->u.n.n = 1+rcp-in_cp;
                    if (*rcp != '\n')
                        break;
                    BSNL();
                }
                RETURN(rcp - in_cp, 0, LEX_SPACE, buf);
            /* punctuations */
            case '!':    /* != !  !\[= ] */
                if (*rcp == '=')
                    RETURN(1, 1, LEX_NEQ, "!=");
                if (*rcp != '\n')
                    RETURN(0, 0, '!', "!");
                BSNL();
                if (*rcp == '=')
                    RETURN(1, 0, LEX_NEQ, "!=");
                RETURN(0, 0, '!', "!");
            case '#':    /* ## #  #[\ ][??= # ] */
                if (*rcp == '#')
                    RETURN(1, 1, LEX_DSHARP, "##");
                if (*rcp != '\n' && *rcp != '?')
                    RETURN(0, 0, LEX_SHARP, "#");
                pr = bsnl3();
                if (pr[0].c == '#')
                    RETSET(0, LEX_DSHARP, "##");
                RETURN(0, 0, LEX_SHARP, "#");
            case '%':    /* %= %> %:%: %: %  %\[= > : ] %\:[\ ]%[\ ]: */
                if (*rcp == '=')
                    RETURN(1, 1, LEX_CREM, "%=");
                if (*rcp == '>')
                    RETURN(1, 1, '}', "%>");
                if (*rcp == ':') {
                    if (rcp[1] == '%' && rcp[2] == ':')
                        RETURN(3, 3, LEX_DSHARP, "%:%:");
                    if (rcp[1] != '\n' && rcp[1] != '?')
                        RETURN(1, 1, LEX_SHARP, "%:");
                } else if (*rcp != '\n' && *rcp != '?')
                    RETURN(0, 0, '%', "%");
                pr = bsnl3();
                if (pr[0].c == '=')
                    RETSET(0, LEX_CREM, "%=");
                if (pr[0].c == '>')
                    RETSET(0, '}', "%>");
                if (pr[0].c == ':') {
                    if (pr[1].c == '%' && pr[2].c == ':')
                        RETSET(2, LEX_DSHARP, "%:%:");
                    RETSET(0, LEX_SHARP, "%:");
                }
                RETURN(0, 0, '%', "%");
        }
    }
}

/* end of lex.c */
