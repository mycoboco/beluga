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
#define NEWBUF() (bsize=IN_MAXTOKEN, pbuf=buf=MEM_CALLOC(1, bsize), ptok->f.alloc = 1)

/* handles escaped newlines */
#define BSNL(y, x)                  \
    do {                            \
        do {                        \
            putbuf('\n');           \
            (y)++;                  \
        } while(*++rcp == '\n');    \
        (x) = 1;                    \
    } while(0)


/* macros to return a token */
#define RETURN(i, s)          \
    do {                      \
        ptok->id = (i);       \
        ptok->spell = (s);    \
        return ptok;          \
    } while(0)

#define RETADJ(x, i, s)              \
    do {                             \
        wx += (x);                   \
        ptok->id = (i);              \
        ptok->spell = (s);           \
        ptok->pos->u.n.dx += (x);    \
        in_cp += (x);                \
        return ptok;                 \
    } while(0)

#define RETDRT(i, s) if (unclean(ptok, (i), (s))) return ptok
#define RETFNL(i)    do { unclean(ptok, (i), ""); return ptok; } while(0)


int lex_inc = 1;    /* true while parsing #include */


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
 *  recognizes "unclean" tokens;
 *  ASSUMPTION: warning for trigraphs issued once per file
 */
static int unclean(lex_t *ptok, int id, const char *s)
{
    int c;
    int y = 0;
    sz_t x = wx;
    register const char *rcp = in_cp;

    assert(ptok);
    assert(s);

    pbuf = buf+1, *pbuf = '\0';
    for (; *s != '\0'; s++) {
        if (*rcp == '\n')
            BSNL(y, x);
        c = *rcp++, x++;
        if (main_opt()->trigraph && c == '?' && rcp[0] == '?' && (c = conv3(rcp[1])) != '?' &&
            (in_trigraph(rcp-1), main_opt()->trigraph & 1) && c == *s) {
            putbuf('?');
            putbuf('?');
            putbuf(rcp[1]);
            rcp += 2;
            x += 2;
        } else if (c == *s)
            putbuf(c);
        else
            return 0;
    }

    dy += y;
    wx = x;
    in_cp = rcp;

    ptok->id = id;
    ptok->spell = buf;
    ptok->f.clean = (buf[1] == '\0');
    ptok->pos->u.n.dy = y;
    ptok->pos->u.n.dx = x;

    return 1;
}


/*
 *  recognizes string literals
 */
static void scon(int q, lex_t *ptok)
{
    int c;
    int wide = (buf[0] == 'L');
    register const char *rcp = in_cp + wide;

    while (*rcp != q && *rcp != '\0') {
        if (*rcp == '\n') {
            ptok->f.clean = 0;
            BSNL(dy, wx);
            ptok->pos->u.n.dy = dy;
            in_cp = rcp;
            continue;
        }
        c = *rcp++;
        if (c == '\\') {
            putbuf(c);
            c = *rcp;
            if (c != '\0')
                rcp++;
        } else if (main_opt()->trigraph && c == '?' && rcp[0] == '?' && conv3(rcp[1]) != '?') {
            in_trigraph(rcp-1);
            if (main_opt()->trigraph & 1)
                ptok->f.clean = 0;
        }
        putbuf(c);
    }
    wx = in_getwx(wx, in_cp, rcp, NULL)+1;
    ptok->pos->u.n.dx = wx;
    in_cp = rcp + 1;
    if (*rcp == q)
        putbuf(q);
    else {
        in_cp--, wx--;
        ptok->pos->u.n.dx--;
        err_issue(ptok->pos, ERR_PP_UNCLOSESTR, q);
    }
    if (q == '\'' && pbuf-buf == 2+wide)
        err_issue(ptok->pos, ERR_PP_EMPTYCHAR);
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
    ptok->f.alloc = 0;
    ptok->f.clean = 1;
    ptok->pos = lmap_add(dy, wx);

    while (1) {
        register const char *rcp = in_cp;

        in_cp = rcp + 1, wx++;
        switch(*rcp++) {
            /* whitespaces */
            case '\n':    /* line splicing */
                dy++;
                ptok->pos->u.n.py++;
                ptok->pos->u.n.wx = wx = 1;
                ptok->pos->u.n.dx = 1+1;
                break;
            case '\0':    /* line end */
            newline:
                /* EOI is detected with unusual check
                   because newline constitutes valid token */
                dy = 0, wx = 1;
                if (in_cp > in_limit) {
                    in_cp = in_limit;
                    RETURN(LEX_EOI, "");
                } else {
                    assert(!fromstr);
                    in_nextline();
                    RETURN(LEX_NEWLINE, "");
                }
            case '\v':    /* ISCH_SP() */
            case '\f':
            case '\r':
            case ' ':
            case '\t':
                NEWBUF();
                rcp--;
                while (1) {
                    do {
                        putbuf(*rcp++);
                    } while(ISCH_SP(*rcp));
                    ptok->pos->u.n.dx += rcp-in_cp;
                    in_cp = rcp;
                    if (*rcp != '\n') {
                        wx = ptok->pos->u.n.dx;
                        RETURN(LEX_SPACE, buf);
                    }
                    BSNL(dy, wx);
                    if (!ISCH_SP(*rcp)) {
                        pbuf[in_cp-rcp] = '\0';
                        in_cp = rcp;
                        break;
                    }
                    in_cp = rcp;
                    ptok->f.clean = 0;
                    ptok->pos->u.n.dy = dy;
                    ptok->pos->u.n.dx = 1;
                }
                RETURN(LEX_SPACE, buf);
            /* punctuations */
            case '!':    /* != !  !\[= ] */
                if (*rcp == '=')
                    RETADJ(1, LEX_NEQ, "!=");
                if (*rcp != '\n')
                    RETURN('!', "!");
                NEWBUF();
                putbuf('!');
                RETDRT(LEX_NEQ, "=");
                RETFNL('!');
            case '"':    /* string literals and header */
                NEWBUF();
                putbuf('"');
            strlit:
                scon(rcp[-1], ptok);
                RETURN(LEX_SCON, buf);
            case '#':    /* ## #  #[\ ][??= # ] */
                if (*rcp == '#')
                    RETADJ(1, LEX_DSHARP, "##");
                if (*rcp != '\n' && *rcp != '?')
                    RETURN(LEX_SHARP, "#");
                NEWBUF();
                putbuf('#');
                RETDRT(LEX_DSHARP, "#");
                RETFNL(LEX_SHARP);
            case '%':    /* %= %> %:%: %: %  %\[= > : ] %\:[\ ]%[\ ]: */
                if (*rcp == '=')
                    RETADJ(1, LEX_CREM, "%=");
                if (*rcp == '>')
                    RETADJ(1, '}', "%>");
                if (*rcp == ':') {
                    if (rcp[1] == '%' && rcp[2] == ':')
                        RETADJ(3, LEX_DSHARP, "%:%:");
                    if (rcp[1] != '\n')
                        RETADJ(1, LEX_SHARP, "%:");
                } else if (*rcp != '\n')
                    RETURN('%', "%");
                NEWBUF();
                putbuf('%');
                RETDRT(LEX_CREM, "=");
                RETDRT('}', ">");
                RETDRT(LEX_DSHARP, ":%:");
                RETDRT(LEX_SHARP, ":");
                RETFNL('%');
            case '&':    /* && &= & */
                if (*rcp == '&')
                    RETADJ(1, LEX_ANDAND, "&&");
                if (*rcp == '=')
                    RETADJ(1, LEX_CBAND, "&=");
                if (*rcp != '\n')
                    RETURN('&', "&");
                NEWBUF();
                putbuf('&');
                RETDRT(LEX_ANDAND, "&");
                RETDRT(LEX_CBAND, "=");
                RETFNL('&');
            case '\'':    /* character constant */
                NEWBUF();
                putbuf('\'');
                goto strlit;
            case 'L':    /* L'x' L"x" ids */
                if (*rcp == '\'' || *rcp == '"') {
                    NEWBUF();
                    putbuf('L');
                    putbuf(*rcp++);
                    goto strlit;
                }
                /* no break */
            default:    /* unknown chars */
                NEWBUF();
                putbuf(rcp[-1]);
                RETURN(LEX_UNKNOWN, buf);
        }
    }

    /* assert(!"impossible control flow -- should never reach here");
       RETURN(LEX_EOI, ""); */
}

/* end of lex.c */
