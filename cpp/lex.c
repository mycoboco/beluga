/*
 *  lexical analyzer for preprocessor
 */

#include <ctype.h>         /* isdigit, isprint, tolower */
#include <stddef.h>        /* size_t, NULL */
#include <stdio.h>         /* sprintf */
#include <string.h>        /* strlen, memcpy */
#include <cbl/arena.h>     /* ARENA_ALLOC, ARENA_CALLOC */
#include <cbl/assert.h>    /* assert */

#include "../src/alist.h"
#include "../src/common.h"
#include "../src/err.h"
#include "../src/in.h"
#include "strg.h"
#include "util.h"
#include "lex.h"

/* puts character into token buffer */
#define putbuf(c) (((pbuf == buf+bsize-1)? resize(): (void)0), *pbuf++ = (c))

/* allocates buffer for token spelling;
   cannot reuse unused one from previous call because of arena slot change */
#define NEWBUF() (bsize=IN_MAXTOKEN, pbuf=buf=ARENA_CALLOC(*strg_tok, 1, bsize))

/* returns token adjusting in_cp */
#define RETURN(c, i, r)             \
    do {                            \
        in_cp += c;                 \
        ptok->id = (i);             \
        ptok->rep = (char *)(r);    \
        ptok->blue = 0;             \
        return ptok;                \
    } while(0)


int lex_inc;           /* true while parsing #include */
int lex_direc;         /* true while parsing directives */
lex_pos_t lex_cpos;    /* locus of current token */


static int fromstr;                  /* true while input coming from string */
static const lex_pos_t *posstr;      /* locus for tokens from string */
static size_t bsize;                 /* size of token buffer */
static unsigned char *buf, *pbuf;    /* pointers to maintain token buffer */


/*
 *  enlarges a token buffer
 */
static void resize(void)
{
    unsigned char *pold;

    pold = buf;
    buf = ARENA_CALLOC(*strg_tok, bsize+IN_MAXTOKEN, 1);
    memcpy(buf, pold, bsize);
    pbuf = buf + bsize - 1;
    bsize += IN_MAXTOKEN;
}


/*
 *  recognizes string literals
 */
static void scon(int q)
{
    int c = 0;

    while (*in_cp != q && *in_cp != '\n' && *in_cp != '\0') {
        c = *in_cp++;
        if (c == '\\') {
            putbuf(c);
            c = *in_cp;
            if (c != '\n')
                in_cp++;
        }
        putbuf(c);
    }
    if (*in_cp == q) {
        putbuf(q);
        in_cp++;
    } else if (!fromstr)
        err_issue(ERR_PP_UNCLOSESTR, q);
    else
        err_issuep(posstr, ERR_PP_UNCLOSESTR, q);
    if (q == '\'' && pbuf - buf == 2 + (buf[0] == 'L'))
        err_issuep((fromstr)? posstr: &lex_cpos, ERR_PP_EMPTYCHAR);
}


/*
 *  recognizes pp-numbers
 */
static void ppnum(void)
{
    unsigned char c;

    do {
        c = *in_cp;
        putbuf(c);
        in_cp++;
    } while(ISCH_IP(*in_cp) || ((*in_cp == '-' || *in_cp == '+') && tolower(c) == 'e'));
}


/*
 *  recognizes identifiers
 */
static void id(void)
{
    register const unsigned char *rcp = in_cp;

    while (ISCH_I(*rcp))
        putbuf(*rcp++);

    in_cp = rcp;
}


/*
 *  recognizes header names
 */
static int header(int c)
{
    register const unsigned char *rcp = in_cp;

    assert(c == '"' || c == '<');

    putbuf(c);
    if (c == '<')
        c = '>';
    while (*rcp != c && *rcp != '\n')
        putbuf(*rcp++);
    if (*rcp == c) {
        putbuf(c);
        in_cp = ++rcp;
        return 1;
    } else if (c == '"') {
        assert(!fromstr);
        err_issue(ERR_PP_UNCLOSEHDR, c);
        return 1;
    }

    return 0;
}


/*
 *  retrieves a token from the input stream
 */
lex_t *(lex_nexttok)(void)
{
    static lex_pos_t pos;

    lex_t *ptok;

    assert(in_cp);
    assert(in_limit);

    ptok = ARENA_ALLOC(*strg_tok, sizeof(*ptok));

    while (1) {
        register const unsigned char *rcp = in_cp;

        lex_cpos.c = in_cpos.c;
        lex_cpos.fy = in_cpos.fy;
        lex_cpos.f = in_cpos.f;
        lex_cpos.y = in_cpos.y;
        lex_cpos.x = rcp-in_line + in_outlen + 1;

        in_cp = rcp + 1;
        switch(*rcp++) {
            /* whitespaces */
            case '\n':
            newline:
                /* unlike compiler proper, EOI is detected with unusual check
                   because newline constitutes valid token */
                if (in_cp > in_limit) {
                    in_cp = in_limit;
                    RETURN(0, LEX_EOI, &pos);
                } else {
                    lex_pos_t *ppos = ARENA_ALLOC(*strg_tok, sizeof(*ppos));
                    assert(!fromstr);
                    *ppos = lex_cpos;
                    in_nextline();
                    RETURN(0, LEX_NEWLINE, ppos);
                }
            case '\v':
            case '\f':
            case '\r':
            case ' ':
            case '\t':
                NEWBUF();
                rcp--;
                do {
                    if (lex_direc && *rcp != ' ' && *rcp != '\t' && !fromstr)
                        in_cp = rcp, err_issue(ERR_PP_SPHTDIREC);
                    putbuf(*rcp++);
                } while (*rcp == ' ' || *rcp == '\t' || *rcp == '\v' || *rcp == '\f' ||
                         *rcp == '\r');
                in_cp = rcp;
                RETURN(0, LEX_SPACE, buf);
            /* punctuations */
            case '!':    /* != and ! */
                if (*rcp == '=')
                    RETURN(1, LEX_NEQ, "!=");
                RETURN(0, '!', "!");
            case '"':    /* string literal and header */
                if (lex_inc)
                    goto header;
                NEWBUF();
                putbuf('"');
            strlit:
                scon(in_cp[-1]);
                RETURN(0, LEX_SCON, buf);
            case '#':
                if (*rcp == '#')
                    RETURN(1, LEX_DSHARP, "##");
                RETURN(0, LEX_SHARP, "#");
            case '&':    /* && and & */
                if (*rcp == '&')
                    RETURN(1, LEX_ANDAND, "&&");
                RETURN(0, '&', "&");
            case '\'':    /* character constant */
                NEWBUF();
                putbuf('\'');
                goto strlit;
            case '+':    /* ++ and + */
                if (*rcp == '+')
                    RETURN(1, LEX_INCR, "++");
                RETURN(0, '+', "+");
            case '-':    /* ->, -- and - */
                if (*rcp == '>')
                    RETURN(1, LEX_DEREF, "->");
                if (*rcp == '-')
                    RETURN(1, LEX_DECR, "--");
                RETURN(0, '-', "-");
            case '.':    /* ..., . and pp-numbers */
                if (rcp[0] == '.' && rcp[1] == '.')
                    RETURN(2, LEX_ELLIPSIS, "...");
                if (!isdigit(*rcp))
                    RETURN(0, '.', ".");
                NEWBUF();
                in_cp = rcp - 1;
                ppnum();
                RETURN(0, LEX_PPNUM, buf);
            case '/':    /* comments, //-comments and / */
                if (*rcp == '*' && !fromstr) {    /* comments */
                    int c = 0;
                    rcp++;    /* skips * */
                    while (!(c == '*' && *rcp == '/')) {
                        if (c == '/' && *rcp == '*') {
                            in_cp = rcp - 1;
                            err_issue(ERR_PP_CMTINCMT);
                        }
                        if (*rcp == '\n') {
                            c = *rcp;
                            in_cp = rcp + 1;
                            in_nextline();
                            rcp = in_cp;
                            if (rcp == in_limit)
                                break;
                        } else
                            c = *rcp++;
                    }
                    in_cp = rcp;
                    if (in_cp < in_limit)
                        in_cp++;
                    else
                        err_issue(ERR_PP_UNCLOSECMT);
                    RETURN(0, LEX_SPACE, " ");
                } else if (*rcp == '/' && !fromstr) {
                    if (main_opt()->std == 1)
                        in_cp--, err_issue(ERR_PP_C99CMT), in_cp++;
                    else if (!fromstr)    /* //-comments supported */
                        goto newline;
                }
                RETURN(0, '/', "/");
            case '<':    /* <=, <<, < and header */
                if (lex_inc) {
            header:
                    NEWBUF();
                    if (header(in_cp[-1]))
                        RETURN(0, LEX_HEADER, buf);
                }
                switch(*rcp) {
                    case '=':
                        RETURN(1, LEX_LEQ, "<=");
                    case '<':
                        RETURN(1, LEX_LSHFT, "<<");
                    case ':':
                        RETURN(1, '[', "<:");
                    case '%':
                        RETURN(1, '{', "<%");
                }
                RETURN(0, '<', "<");
            case '=':    /* == and = */
                if (*rcp == '=')
                    RETURN(1, LEX_EQEQ, "==");
                RETURN(0, '=', "=");
            case '>':    /* >=, >> and > */
                if (*rcp == '=')
                    RETURN(1, LEX_GEQ, ">=");
                if (*rcp == '>')
                    RETURN(1, LEX_RSHFT, ">>");
                RETURN(0, '>', ">");
            case '|':    /* || and | */
                if (*rcp == '|')
                    RETURN(1, LEX_OROR, "||");
                RETURN(0, '|', "|");
            case '%':
                if (*rcp == '>')
                    RETURN(1, '}', "%>");
                else if (*rcp == ':') {
                    if (rcp[1] == '%' && rcp[2] == ':')
                        RETURN(3, LEX_DSHARP, "%:%:");
                    RETURN(1, LEX_SHARP, "%:");
                }
                RETURN(0, '%', "%");
            case ':':
                if (*rcp == '>')
                    RETURN(1, ']', ":>");
                RETURN(0, ':', ":");
            /* one-char tokens */
            case '(':
                RETURN(0, '(', "(");
            case ')':
                RETURN(0, ')', ")");
            case '*':
                RETURN(0, '*', "*");
            case ',':
                RETURN(0, ',', ",");
            case ';':
                RETURN(0, ';', ";");
            case '?':
                RETURN(0, '?', "?");
            case '~':
                RETURN(0, '~', "~");
            case '[':
                RETURN(0, '[', "[");
            case ']':
                RETURN(0, ']', "]");
            case '^':
                RETURN(0, '^', "^");
            case '{':
                RETURN(0, '{', "{");
            case '}':
                RETURN(0, '}', "}");
            case 'L':    /* L'x', L"x" and ids */
                if (*rcp == '\'' || *rcp == '"') {
                    NEWBUF();
                    putbuf('L');
                    putbuf(*rcp++);
                    in_cp = rcp;
                    goto strlit;
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
                NEWBUF();
                in_cp = rcp - 1;
                id();
                RETURN(0, LEX_ID, buf);
            case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7':
            case '8': case '9':    /* pp-numbers */
                NEWBUF();
                in_cp = rcp - 1;
                ppnum();
                RETURN(0, LEX_PPNUM, buf);
            case '\0':
                assert(fromstr);
                in_cp = in_limit;
                RETURN(0, LEX_EOI, &pos);
            default:
                NEWBUF();
                putbuf(rcp[-1]);    /* unknown chars */
                RETURN(0, LEX_UNKNOWN, buf);
        }
    }
}


/*
 *  tokenizes a string
 */
alist_t *(lex_run)(const char *s, const lex_pos_t *ppos)
{
    static lex_t space = {
        LEX_SPACE,
        0,
        " "
    };

    const unsigned char *pcp;
    const unsigned char *plimit;
    const unsigned char *pline;
    lex_pos_t cpos;
    lex_t *t;
    alist_t *list = NULL;

    assert(s);
    assert(ppos);

    pcp = in_cp;
    plimit = in_limit;
    pline = in_line;
    cpos = lex_cpos;
    in_line = in_cp = (unsigned char *)s;
    in_limit = (unsigned char *)(s + strlen(s));
    fromstr = 1;
    posstr = ppos;

    if ((t=skip(lex_nexttok(), lex_nexttok))->id != LEX_EOI) {
        list = alist_append(list, t, strg_line);
        while ((t=skip(lex_nexttok(), lex_nexttok))->id != LEX_EOI)
            list = alist_append(alist_append(list, &space, strg_line), t, strg_line);
    }

    in_cp = pcp;
    in_limit = plimit;
    in_line = pline;
    lex_cpos = cpos;
    fromstr = 0;
    posstr = NULL;

    return list;
}


/*
 *  recognizes an escape sequence
 */
unsigned long (lex_bs)(const char **pp, unsigned long lim, const lex_pos_t *ppos, const char *w)
{
    int c;
    int ovf;
    unsigned long n;
    char m[] = "\\x\0";
    const char *hex = "0123456789abcdef";

    assert(pp);
    assert(*pp);
    assert(ppos);
    assert(w);

    switch(*(*pp)++) {
        case 'a':
            return '\a';
        case 'b':
            return '\b';
        case 'f':
            return '\f';
        case 'n':
            return '\n';
        case 'r':
            return '\r';
        case 't':
            return '\t';
        case 'v':
            return '\v';
        case '\'':
        case '"':
        case '\\':
        case '\?':
            break;
        case 'x':    /* \xh...h */
            ovf = 0;
            if (!isxdigit(**pp)) {
                m[2] = **pp;
                if (isprint(*(unsigned char *)*pp))
                    err_issuep(ppos, ERR_PP_INVESC1, m, w);
                else
                    err_issuep(ppos, ERR_PP_INVESC2, w);
                return 0;
            }
            c = n = 0;
            do {
                c = strchr(hex, tolower(*(*pp)++)) - hex;
                if (n & ~(lim >> 4))
                    ovf = 1;
                else
                    n = (n << 4) + c;
            } while(isxdigit(**pp));
            if (ovf) {
                err_issuep(ppos, ERR_PP_LARGEHEX);
                n = lim;
            }
            return n & lim;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
            c = 1;
            n = (*pp)[-1] - '0';
            if (**pp >= '0' && **pp <= '7') {
                n = (n << 3) + (*(*pp)++ - '0'), c++;
                if (**pp >= '0' && **pp <= '7')
                    n = (n << 3) + (*(*pp)++ - '0'), c++;
            }
            if (isdigit((*pp)[0])) {
                if (c < 3 && ((*pp)[0] == '8' || (*pp)[0] == '9'))
                    err_issuep(ppos, ERR_PP_ESCOCT89);
                else if (c == 3)
                    err_issuep(ppos, ERR_PP_ESCOCT3DIG);
            }
            if (n > lim) {
                err_issuep(ppos, ERR_PP_LARGEOCT);
                n = lim;
            }
            return n & lim;
        default:
            m[1] = (*pp)[-1];
            if (isprint(((unsigned char *)(*pp))[-1]))
                err_issuep(ppos, ERR_PP_INVESC1, m, w);
            else
                err_issuep(ppos, ERR_PP_INVESC2, w);
            break;
    }

    return (*pp)[-1];
}


/*
 *  converts a locus to a string;
 *  should not be invoked more than once in the same call;
 *  cannot use snbuf() to avoid interleave;
 *  copied from beluga's lex_outpos()
 */
const char *(lex_outpos)(const lex_pos_t *src)
{
    static char buf[80];

    size_t len;
    char *pbuf = buf;

    assert(src);
    assert(src->f);

    len = strlen(src->f) + 2 + BUFN*2 + 1;    /* file:y:x */
    if (sizeof(buf) < len)
        pbuf = ARENA_ALLOC(strg_line, len);
    sprintf(pbuf, "%s:%lu:%lu", src->f, src->y, src->x);

    return pbuf;
}

/* end of lex.c */
