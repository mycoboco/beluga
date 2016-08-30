/*
 *  primitive lexical analyzer
 */

#include <stddef.h>        /* size_t */
#include <string.h>        /* memset */
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
#define NEWBUF(c)                   \
    (bsize = IN_MAXTOKEN,           \
     buf = MEM_CALLOC(1, bsize),    \
     buf[0] = (c),                  \
     pbuf = buf + 1,                \
     ptok->f.alloc = 1)

/* handles escaped newlines */
#define BSNL(x)                     \
    do {                            \
        do {                        \
            putbuf('\n');           \
            y++;                    \
        } while(*++rcp == '\n');    \
        (x) = 1;                    \
    } while(0)

/* sets token properties */
#define SETTOK(i, c, y, x)       \
    (ptok->id = (i),             \
     ptok->spell = buf,          \
     ptok->f.clean = (c),        \
     ptok->pos->u.n.dy = (y),    \
     ptok->pos->u.n.dx = (x))

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
#define RETFNL(i)    do { unclean(ptok, (i), "\0"); return ptok; } while(0)


int lex_inc;      /* true while parsing #include */
int lex_direc;    /* true while parsing directives */


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
    memset(pbuf, 0, IN_MAXTOKEN+1);
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

    pbuf = buf + ((buf[0] == '?')? 3: 1);
    for (s++; *s != '\0'; s++) {
        if (*rcp == '\n')
            BSNL(x);
        c = *rcp++, x++;
        if (c == '?' && rcp[0] == '?' && main_opt()->trigraph &&
            (c = in_trigraph(rcp-1)) != '?' && (main_opt()->trigraph & 1) && c == *s) {
            putbuf('?');
            putbuf('?');
            putbuf(rcp[1]);
            rcp += 2, x += 2;
        } else if (c == *s)
            putbuf(c);
        else
            return 0;
    }
    *pbuf = '\0';    /* token buffer reused */

    dy += y;
    wx = x;
    in_cp = rcp;
    SETTOK(id, (buf[1] == '\0'), y, x);

    return 1;
}


/*
 *  recognizes string literals
 */
static void scon(lex_t *ptok)
{
    int q, c;
    int y = 0;
    int w = (buf[0] == 'L');
    register const char *rcp = in_cp;

    assert(ptok);

    if (w) {
        if (*rcp == '\n')
            BSNL(wx);
        q = *rcp++;
        putbuf(q);
    } else
        q = buf[0];
    assert(q == '\'' || q == '"');

    while (*rcp != q && *rcp != '\0') {
        if (*rcp == '\n') {
            ptok->f.clean = 0;
            BSNL(wx);
            continue;
        }
        c = *rcp++;
        if (c == '\\') {
            putbuf(c);
            if (*rcp == '\n') {
                ptok->f.clean = 0;
                BSNL(wx);
            }
            c = *rcp;
            if (c != '\0')
                rcp++;
        }
        if (c == '?' && rcp[0] == '?' && main_opt()->trigraph && in_trigraph(rcp-1) != '?' &&
            (main_opt()->trigraph & 1))
            ptok->f.clean = 0;
        putbuf(c);
    }
    ptok->pos->u.n.dy = y, dy += y;
    ptok->pos->u.n.dx = wx = in_getwx(wx, in_cp, rcp, NULL)+1;
    in_cp = rcp + 1;
    putbuf(q);
    if (*rcp != q) {
        in_cp--, ptok->pos->u.n.dx--, wx--;
        err_issue(ptok->pos, ERR_PP_UNCLOSESTR, q);
    } else if (q == '\'' && (buf[w+1] == '\'' || buf[w+1] == '\n')) {
        for (pbuf = &buf[w+1]; *pbuf == '\n'; pbuf++)
            continue;
        if (*pbuf == '\'')
            err_issue(ptok->pos, ERR_PP_EMPTYCHAR);
    }
}


/*
 *  recognizes comments
 */
static int comment(lex_t *ptok)
{
    int c;
    int y = 0;
    sz_t x = wx;
    register const char *rcp = in_cp;
    const char *slash = rcp - 1;

    assert(ptok);

    if (*rcp == '\n') {
        do { y++; } while(*++rcp == '\n');
        x = 1;
    }
    if (*rcp == '*') {    /* block comments */
        c = 0;
        dy += y, wx = x;
        rcp++;    /* skips * */
        while (!(c == '*' && *rcp == '/')) {
            if (*rcp == '\0') {
                c = 0;
                y++, dy = 0, wx = 1;
                in_nextline();
                rcp = in_cp;
                if (rcp == in_limit)
                    break;
                continue;
            }
            if (*rcp == '\n') {
                do { dy++, y++; } while(*++rcp == '\n');
                wx = 1;
                continue;
            }
            if (*rcp == '/')
                slash = rcp;
            else if (c == '/' && *rcp == '*')
                err_issuel(slash, 2, ERR_PP_CMTINCMT);
            c = *rcp++;
        }
        ptok->pos->u.n.dy = y;
        if (rcp < in_limit) {
            rcp++;    /* skips / */
            ptok->pos->u.n.dx = wx = in_getwx(wx, in_cp, rcp, NULL);
            in_cp = rcp;
            return 1;    /* returns space */
        } else {
            ptok->pos->u.n.dx = wx;
            err_issue(ptok->pos, ERR_PP_UNCLOSECMT);
            return 2;    /* returns newline */
        }
    }
    if (*rcp == '/') {
        if (main_opt()->std != 1) {    /* line comments supported */
            dy += y, wx = x;
            while (*++rcp != '\0') {
                if (*rcp == '\n') {
                    do { dy++, y++; } while(*++rcp == '\n');
                    wx = 1;
                }
            }
            ptok->pos->u.n.dy = y;
            ptok->pos->u.n.dx = wx = in_getwx(wx, in_cp, rcp, NULL);
            in_cp = rcp;
            return 1;    /* returns space */
        } else
            err_issuel(slash, 2, ERR_PP_C99CMT);
    }
    return 0;    /* not comments */
}


/*
 *  recognizes pp-numbers
 */
static void ppnum(lex_t *ptok)
{
    int c;
    int y = 0;
    register const char *rcp = in_cp;

    assert(ptok);

    while (1) {
        while(ISCH_IP(*rcp) || ((*rcp == '-' || *rcp == '+') && tolower(c) == 'e')) {
            c = *rcp++;
            putbuf(c);
        }
        ptok->pos->u.n.dx += rcp-in_cp;
        in_cp = rcp;
        if (*rcp != '\n') {
            wx = ptok->pos->u.n.dx;
            return;
        }
        BSNL(wx);
        if (!(ISCH_IP(*rcp) || ((*rcp == '-' || *rcp == '+') && tolower(c) == 'e'))) {
            pbuf[in_cp-rcp] = '\0';
            in_cp = rcp;
            dy += y;
            break;
        }
        in_cp = rcp;
        ptok->f.clean = 0;
        ptok->pos->u.n.dy = y, dy += y;
        ptok->pos->u.n.dx = 1;
    }
}


/*
 *  recognizes identifiers
 */
static void id(lex_t *ptok)
{
    int y = 0;
    register const char *rcp = in_cp;

    assert(ptok);

    while (1) {
        while(ISCH_I(*rcp))
            putbuf(*rcp++);
        ptok->pos->u.n.dx += rcp-in_cp;
        in_cp = rcp;
        if (*rcp != '\n') {
            wx = ptok->pos->u.n.dx;
            return;
        }
        BSNL(wx);
        if (!ISCH_I(*rcp)) {
            pbuf[in_cp-rcp] = '\0';
            in_cp = rcp;
            dy += y;
            break;
        }
        in_cp = rcp;
        ptok->f.clean = 0;
        ptok->pos->u.n.dy = y, dy += y;
        ptok->pos->u.n.dx = 1;
    }
}


/*
 *  recognizes header names
 */
static int header(lex_t *ptok)
{
    int c;
    int y = 0;
    sz_t x = wx;
    int clean = 1;
    int q = buf[0];
    const char *incp = in_cp;
    register const char *rcp = incp;

    assert(q == '"' || q == '<');
    assert(ptok);

    if (q == '<')
        q = '>';
    while (*rcp != q && *rcp != '\0') {
        if (*rcp == '\n') {
            clean = 0;
            BSNL(x);
            continue;
        }
        c = *rcp++;
        if (c == '?' && rcp[0] == '?' && main_opt()->trigraph && in_trigraph(rcp-1) != '?' &&
            (main_opt()->trigraph & 1))
            clean = 0;
        putbuf(c);
    }

    if (*rcp == q) {
        putbuf(q);
        dy += y;
        in_cp = ++rcp;
        wx = in_getwx(x, incp, rcp, NULL);
        SETTOK(LEX_HEADER, clean, y, wx);
        return 1;
    } else if (q == '"') {
        putbuf(q);
        dy += y;
        in_cp = rcp;
        wx = in_getwx(x, incp, rcp, NULL);
        SETTOK(LEX_HEADER, clean, y, wx);
        err_issue(ptok->pos, ERR_PP_UNCLOSEHDR, q);
        return 1;
    }

    return 0;
}


/*
 *  looks ahead a character after line splicing
 */
static int getchr(void)
{
    register const char *p = in_cp;

    while (*p == '\n')
        p++;

    return *p;
}


/*
 *  retrieves a token from the input stream
 */
lex_t *(lex_next)(void)
{
    lex_t *ptok;

    assert(in_cp);
    assert(in_limit);

    ptok = MEM_ALLOC(sizeof(*ptok));
    ptok->f.alloc = 0;
    ptok->f.clean = 1;
    ptok->pos = lmap_add(dy, wx);
    ptok->next = ptok;

    while (1) {
        register const char *rcp = in_cp;

        in_cp = rcp + 1, wx++;
        switch(*rcp++) {
            /* whitespaces */
            case '\n':    /* line splicing */
                ptok->pos->u.n.py++, dy++;
                ptok->pos->u.n.wx = wx = 1;
                ptok->pos->u.n.dx = 1+1;
                break;
            case '\0':    /* line end */
                /* EOI is detected with unusual check
                   because newline constitutes valid token */
                dy = 0, wx = 1;
                if (in_cp > in_limit) {
                    in_cp = in_limit;
                    RETURN(LEX_EOI, "");
                } else {
                    assert(!fromstr);
                    in_nextline();
                    RETURN(LEX_NEWLINE, "\n");
                }
            case '\v':    /* ISCH_SP() */
            case '\f':
            case '\r':
            case ' ':
            case '\t':
                NEWBUF(rcp[-1]);
                while (1) {
                    int y = 0;
                    while(ISCH_SP(*rcp))
                        putbuf(*rcp++);
                    ptok->pos->u.n.dx += rcp-in_cp;
                    in_cp = rcp;
                    if (*rcp != '\n') {
                        wx = ptok->pos->u.n.dx;
                        RETURN(LEX_SPACE, buf);
                    }
                    BSNL(wx);
                    if (!ISCH_SP(*rcp)) {
                        pbuf[in_cp-rcp] = '\0';
                        in_cp = rcp;
                        dy += y;
                        break;
                    }
                    in_cp = rcp;
                    ptok->f.clean = 0;
                    ptok->pos->u.n.dy = y, dy += y;
                    ptok->pos->u.n.dx = 1;
                }
                RETURN(LEX_SPACE, buf);
            /* punctuations */
            case '!':    /* != ! */
                if (*rcp == '=')
                    RETADJ(1, LEX_NEQ, "!=");
                if (*rcp != '\n')
                    RETURN('!', "!");
                NEWBUF('!');
                RETDRT(LEX_NEQ, "!=");
                RETFNL('!');
            case '"':    /* string literals and header */
                if (lex_inc)
                    goto header;
                NEWBUF('"');
            strlit:
                scon(ptok);
                RETURN(LEX_SCON, buf);
            case '#':    /* ## # #??= */
                if (*rcp == '#')
                    RETADJ(1, LEX_DSHARP, "##");
                if (*rcp != '\n' && *rcp != '?')
                    RETURN(LEX_SHARP, "#");
                NEWBUF('#');
                RETDRT(LEX_DSHARP, "##");
                RETFNL(LEX_SHARP);
            case '%':    /* %= %> %:%: %: % */
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
                NEWBUF('%');
                RETDRT(LEX_CREM, "%=");
                RETDRT('}', "%>");
                RETDRT(LEX_DSHARP, "%:%:");
                RETDRT(LEX_SHARP, "%:");
                RETFNL('%');
            case '&':    /* && &= & */
                if (*rcp == '&')
                    RETADJ(1, LEX_ANDAND, "&&");
                if (*rcp == '=')
                    RETADJ(1, LEX_CBAND, "&=");
                if (*rcp != '\n')
                    RETURN('&', "&");
                NEWBUF('&');
                RETDRT(LEX_ANDAND, "&&");
                RETDRT(LEX_CBAND, "&=");
                RETFNL('&');
            case '\'':    /* character constant */
                NEWBUF('\'');
                goto strlit;
            case '*':    /* *= * */
                if (*rcp == '=')
                    RETADJ(1, LEX_CMUL, "*=");
                if (*rcp != '\n')
                    RETURN('*', "*");
                NEWBUF('*');
                RETDRT(LEX_CMUL, "*=");
                RETFNL('*');
            case '+':    /* ++ += + */
                if (*rcp == '+')
                    RETADJ(1, LEX_INCR, "++");
                if (*rcp == '=')
                    RETADJ(1, LEX_CADD, "+=");
                if (*rcp != '\n')
                    RETURN('+', "+");
                NEWBUF('+');
                RETDRT(LEX_INCR, "++");
                RETDRT(LEX_CADD, "+=");
                RETFNL('+');
            case '-':    /* -> -- -= - */
                if (*rcp == '>')
                    RETADJ(1, LEX_DEREF, "->");
                if (*rcp == '-')
                    RETADJ(1, LEX_DECR, "--");
                if (*rcp == '=')
                    RETADJ(1, LEX_CSUB, "-=");
                if (*rcp != '\n')
                    RETURN('-', "-");
                NEWBUF('-');
                RETDRT(LEX_DEREF, "->");
                RETDRT(LEX_DECR, "--");
                RETDRT(LEX_CSUB, "-=");
                RETFNL('-');
            case '.':    /* ... . pp-numbers */
                if (rcp[0] == '.' && rcp[1] == '.')
                    RETADJ(2, LEX_ELLIPSIS, "...");
                if (isdigit(*rcp))
                    goto ppnum;
                if (*rcp != '\n' && *rcp != '.')
                    RETURN('.', ".");
                if (*rcp == '\n' && isdigit(getchr()))
                    goto ppnum;
                NEWBUF('.');
                RETDRT(LEX_ELLIPSIS, "...");
                RETFNL('.');
            case '/':    /* block-comments line-comments /= / */
                switch(comment(ptok)) {
                    case 1:
                        RETURN(LEX_SPACE, " ");
                    case 2:
                        RETURN(LEX_NEWLINE, "\n");
                }
                if (*rcp == '=')
                    RETADJ(1, LEX_CDIV, "/=");
                if (*rcp != '\n')
                    RETURN('/', "/");
                NEWBUF('/');
                RETDRT(LEX_CDIV, "/=");
                RETFNL('/');
            case ':':    /* :> : */
                if (*rcp == '>')
                    RETADJ(1, ']', ":>");
                if (*rcp != '\n')
                    RETURN(':', ":");
                NEWBUF(':');
                RETDRT(']', ":>");
                RETFNL(':');
            case '<':    /* header <= << <<= <: <% < */
                if (lex_inc) {
            header:
                    NEWBUF(rcp[-1]);
                    if (header(ptok))
                        return ptok;
                }
                switch (*rcp) {
                    case '=':
                        RETADJ(1, LEX_LEQ, "<=");
                    case '<':
                        if (rcp[1] == '=')
                            RETADJ(2, LEX_CLSHFT, "<<=");
                        if (rcp[1] != '\n')
                            RETADJ(1, LEX_LSHFT, "<<");
                        break;
                    case ':':
                        RETADJ(1, '[', "<:");
                    case '%':
                        RETADJ(1, '{', "<%");
                }
                if (*rcp != '\n' && *rcp != '<')
                    RETURN('<', "<");
                NEWBUF('<');
                RETDRT(LEX_LEQ, "<=");
                RETDRT(LEX_CLSHFT, "<<=");
                RETDRT(LEX_LSHFT, "<<");
                RETDRT('[', "<:");
                RETDRT('{', "<%");
                RETFNL('<');
            case '=':    /* == = */
                if (*rcp == '=')
                    RETADJ(1, LEX_EQEQ, "==");
                if (*rcp != '\n')
                    RETURN('=', "=");
                NEWBUF('=');
                RETDRT(LEX_EQEQ, "==");
                RETFNL('=');
            case '>':    /* >= >>= >> > */
                if (*rcp == '=')
                    RETADJ(1, LEX_GEQ, ">=");
                if (*rcp == '>') {
                    if (rcp[1] == '=')
                        RETADJ(2, LEX_CRSHFT, ">>=");
                    if (rcp[1] != '\n')
                        RETADJ(1, LEX_RSHFT, ">>");
                }
                if (*rcp != '\n' && *rcp != '>')
                    RETURN('>', ">");
                NEWBUF('>');
                RETDRT(LEX_GEQ, ">=");
                RETDRT(LEX_CRSHFT, ">>=");
                RETDRT(LEX_RSHFT, ">>");
                RETFNL('>');
            case '^':    /* ^= ^ */
                if (*rcp == '=')
                    RETADJ(1, LEX_CBXOR, "^=");
                if (*rcp != '\n')
                    RETURN('^', "^");
                NEWBUF('^');
                RETDRT(LEX_CBXOR, "^=");
                RETFNL('^');
            case '|':    /* || |= | |??! */
                if (*rcp == '|')
                    RETADJ(1, LEX_OROR, "||");
                if (*rcp == '=')
                    RETADJ(1, LEX_CBOR, "|=");
                if (*rcp != '\n' && *rcp != '?')
                    RETURN('|', "|");
                NEWBUF('|');
                RETDRT(LEX_OROR, "||");
                RETDRT(LEX_CBOR, "|=");
                RETFNL('|');
            case '?':    /* trigraphs ? */
                {
                    int c;
                    if (rcp[0] == '?' && main_opt()->trigraph &&
                        (c = in_trigraph(rcp-1)) != '?' && (main_opt()->trigraph & 1)) {
                        NEWBUF('?');
                        putbuf('?');
                        putbuf(rcp[1]);
                        in_cp = rcp+2, wx += 2;
                        switch(c) {
                            case '#':
                                RETDRT(LEX_DSHARP, "##");
                                RETFNL(LEX_SHARP);
                            case '\\':
                                RETFNL(LEX_UNKNOWN);
                            case '^':
                                RETDRT(LEX_CBXOR, "^=");
                                RETFNL('^');
                            case '|':
                                RETDRT(LEX_OROR, "||");
                                RETDRT(LEX_CBOR, "|=");
                                RETFNL('|');
                            case '[':
                            case ']':
                            case '{':
                            case '}':
                            case '~':
                                RETFNL(c);
                            default:
                                assert(!"invalid trigraph -- should never reach here");
                                break;
                        }
                    }
                }
                RETURN('?', "?");
            /* one-char tokens */
            case '(':
                RETURN('(', "(");
            case ')':
                RETURN(')', ")");
            case ',':
                RETURN(',', ",");
            case ';':
                RETURN(';', ";");
            case '~':
                RETURN('~', "~");
            case '[':
                RETURN('[', "[");
            case ']':
                RETURN(']', "]");
            case '}':
                RETURN('}', "}");
            case '{':
                RETURN('{', "{");
            case 'L':    /* L'x' L"x" ids */
                if (*rcp == '\'' || *rcp == '"') {
                    NEWBUF('L');
                    goto strlit;
                }
                if (*rcp == '\n') {
                    int c = getchr();
                    if (c == '\'' || c == '"') {
                        NEWBUF('L');
                        goto strlit;
                    }
                }
                /* no break */
            case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h':
            case 'i': case 'j': case 'k': case 'l': case 'm': case 'n': case 'o': case 'p':
            case 'q': case 'r': case 's': case 't': case 'u': case 'v': case 'w': case 'x':
            case 'y': case 'z':    /* ids */
            case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H':
            case 'I': case 'J': case 'K': case 'M': case 'N': case 'O': case 'P': case 'Q':
            case 'R': case 'S': case 'T': case 'U': case 'V': case 'W': case 'X': case 'Y':
            case 'Z':
            case '_':
                NEWBUF(rcp[-1]);
                id(ptok);
                RETURN(LEX_ID, buf);
            case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7':
            case '8': case '9':    /* pp-numbers */
            ppnum:
                NEWBUF(rcp[-1]);
                ppnum(ptok);
                RETURN(LEX_PPNUM, buf);
            default:    /* unknown chars */
                NEWBUF(rcp[-1]);
                if (FIRSTUTF8(*rcp))
                    RETURN(LEX_UNKNOWN, buf);
                do {
                    putbuf(*rcp++);
                } while(!FIRSTUTF8(*rcp));
                ptok->pos->u.n.dx = wx = in_getwx(wx-1, in_cp-1, rcp, NULL);
                in_cp = rcp;
                RETURN(LEX_UNKNOWN, buf);
        }
    }

    /* assert(!"impossible control flow -- should never reach here");
       RETURN(LEX_EOI, ""); */
}

/* end of lex.c */
