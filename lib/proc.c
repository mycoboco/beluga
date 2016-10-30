/*
 *  processing for preprocessor
 */

#include <string.h>        /* strcmp */
#include <cbl/assert.h>    /* assert */
#include <cbl/memory.h>    /* MEM_ALLOC, MEM_RESIZE, MEM_FREE */
#include <cdsl/hash.h>     /* hash_new */

#include "common.h"
#include "err.h"
#include "in.h"
#include "inc.h"
#include "lex.h"
#include "lmap.h"
#include "lst.h"
#include "mcr.h"
#include "strg.h"
#include "util.h"
#include "proc.h"


/* processing states */
enum {
    SINIT,      /* initial or after macro guard */
    SIDIREC,    /* directive after initial */
    SAFTRNL,    /* after newline */
    SDIREC,     /* directive */
    SNORM,      /* normal */
    SIGN        /* ignore */
};

/* directives */
enum {
    DINCLUDE,    /* #include */
    DDEFINE,     /* #define */
    DUNDEF,      /* #undef */
    DIF,         /* #if */
    DIFDEF,      /* #ifdef */
    DIFNDEF,     /* #ifndef */
    DELIF,       /* #elif */
    DELSE,       /* #else */
    DENDIF,      /* #endif */
    DLINE,       /* #line */
    DERROR,      /* #error */
    DPRAGMA      /* #pragma */
};


/* internal functions referenced forwardly */
static int direci(lex_t *);


static int (*directive)(lex_t *) = direci;    /* handles "directive" state */

/* diagnoses extra tokens if set */
static int warnxtra[] = {
    0,    /* #include */
    0,    /* #define */
    1,    /* #undef */
    0,    /* #if */
    1,    /* #ifdef */
    1,    /* #ifndef */
    0,    /* #elif */
    1,    /* #else */
    1,    /* #endif */
    0,    /* #line; check done in dline() */
    0,    /* #error */
    1,    /* #pragma */
    0     /* unknown */
};

/* directive look-up table */
static struct {
    const char *name;
    int code;
} dtab[] = {
    "include", DINCLUDE,
    "define",  DDEFINE,
    "undef",   DUNDEF,
    "if",      DIF,
    "ifdef",   DIFDEF,
    "ifndef",  DIFNDEF,
    "elif",    DELIF,
    "else",    DELSE,
    "endif",   DENDIF,
    "line",    DLINE,
    "error",   DERROR,
    "pragma",  DPRAGMA
};

static int state = SINIT;    /* current state */


/*
 *  makes a range
 */
static lex_t *xtratok(lex_t *t)
{
    const lmap_t *s, *e;

    assert(t);

    s = e = t->pos;
    do {
        t = lst_nexti();
        if (t->id != LEX_SPACE && t->id != LEX_NEWLINE)
            e = t->pos;
    } while(t->id != LEX_NEWLINE);

    err_dpos(lmap_range(s, e), ERR_PP_EXTRATOKEN);

    return t;
}


/*
 *  accepts #include;
 *  cannot use snbuf() because of a call to inc_start()
 */
static lex_t *dinclude(void)
{
    static char buf[64+1];    /* size must be (power of 2) + 1 */

    lex_t *t;
    const lmap_t *hpos;
    const char *inc = NULL;
    char *pbuf = buf, *p;

    lst_assert();
    lex_inc = 1;
    NEXTSP(t);    /* consumes include */
    hpos = t->pos;
    if (t->id == LEX_HEADER) {
        inc = LEX_SPELL(t);
        while (1) {
            NEXTSP(t);    /* consumes header or current token */
            if (t->id == LEX_NEWLINE)
                break;
            assert(t->id != LEX_EOI);
            if (t->id == LEX_ID && !t->f.blue && mcr_expand(t))
                continue;
            t = xtratok(t);
            break;
        }
    } else {
        int st = 0;    /* initial */
        size_t blen, slen;
        const lmap_t *epos = NULL;

        while (t->id != LEX_NEWLINE) {
            assert(t->id != LEX_EOI);
            epos = t->pos;
            if (t->id == LEX_ID && !t->f.blue && mcr_expand(t)) {
                NEXTSP(t);    /* consumes expanded id */
                continue;
            }
            assert(t->id != LEX_NEWLINE);
            switch(st) {
                case 0:    /* initial */
                    hpos = t->pos;
                    switch(t->id) {
                        case LEX_SCON:
                            if (t->spell[0] == 'L' || t->spell[0] == '\'') {
                                default:
                                    SKIPNL(t);
                                    continue;
                            }
                            /* no break */
                        case LEX_HEADER:
                            st = 1;
                            inc = LEX_SPELL(t);
                            break;
                        case '<':
                            assert(pbuf == buf);
                            st = 2;
                            p = pbuf;
                            blen = sizeof(buf) - 1;
                            *p++ = '<';
                            break;
                    }
                    break;
                case 1:    /* extra tokens */
                    t = xtratok(t);
                    continue;
                case 2:    /* < seen */
                    slen = strlen(t->spell);
                    if (slen > blen - (p-pbuf)) {
                        const char *oldp = pbuf;
                        blen += ((slen + NELEM(buf)-2) & ~(size_t)(NELEM(buf)-2));
                        if (pbuf == buf) {
                            pbuf = MEM_ALLOC(blen + 1);
                            strcpy(pbuf, oldp);
                        } else
                            MEM_RESIZE(pbuf, blen + 1);
                        p = pbuf + (p - oldp);
                    }
                    strcpy(p, t->spell);
                    p += slen;
                    if (t->id == '>') {
                        st = 1;
                        inc = pbuf;
                        err_dpos(lmap_range(hpos, t->pos), ERR_PP_COMBINEHDR);
                    }
                    break;
                default:
                    assert(!"invalid state -- should never reach here");
                    break;
            }
            NEXTSP(t);    /* consumes handled token */
        }
        if (inc)
            hpos = lmap_range(hpos, epos);
        else
            err_dpos(hpos, ERR_PP_NOHEADER);
    }

    if (!inc || !inc_start(inc, hpos))
        in_nextline();    /* because lex_inc set */

    if (pbuf != buf)
        MEM_FREE(pbuf);
    lex_inc = 0;
    return t;
}


/*
 *  accepts #undef
 */
static lex_t *dundef(void)
{
    lex_t *t;

    NEXTSP(t);    /* consumes undef */
    if (t->id != LEX_ID) {
        err_dpos(t->pos, ERR_PP_NOMCRID);
        SKIPNL(t);
        return t;
    }
    mcr_del(t);
    NEXTSP(t);    /* consumes id */

    return t;
}


/*
 *  accepts a digit sequence for line number
 */
static int digits(sz_t *pn, lex_t *t)
{
    int ovf = 0;
    sz_t n = 0;
    const char *s;

    assert(pn);
    assert(t);

    s = LEX_SPELL(t);
    while (isdigit(*(unsigned char *)s)) {
        if (n > (UX_MAX-(*s-'0')) / 10 || n > (TL_LINENO_STD-(*s-'0')) / 10)
            ovf = 1;
        n = 10*n + (*s++ - '0');
    }

    if (*s != '\0')
        return 0;

    if (ovf)
        err_dpos(t->pos, ERR_PP_LARGELINE);
    else if (n == 0)
        err_dpos(t->pos, ERR_PP_ZEROLINE);
    if (n == 0)
        n = 1;
    *pn = n;

    return 1;
}


/*
 *  recognizes a string literal including escape sequence
 */
static const char *recstr(lex_t *t)
{
    char *p, *r;
    const char *ss, *s;

    assert(t);

    ss = s = LEX_SPELL(t);
    if (!strchr(s, '\\'))
        return s;

    p = r = ARENA_ALLOC(strg_perm, strlen(s)+1);    /* file names go into strg_perm */
    while (*s) {
        *p = (*s == '\\')? lex_bs(t, ss, &s, UCHAR_MAX, "file name"): *s++;
        p++;
    }

    return r;
}


/*
 *  accepts #line
 */
static lex_t *dline(const lmap_t *pos)
{
    lex_t *t;
    int st = 0;    /* initial */
    sz_t n;
    const char *fn = NULL;

    while (1) {
        NEXTSP(t);    /* consumes line or current token */
        if (t->id == LEX_NEWLINE)
            break;
        assert(t->id != LEX_EOI);
        if (t->id == LEX_ID && !t->f.blue && mcr_expand(t))
            continue;
        if (st == 0) {    /* line number */
            if (t->id != LEX_PPNUM || !digits(&n, t)) {
                err_dpos(t->pos, ERR_PP_ILLLINENO, LEX_SPELL(t));
                return t;
            }
            st = 1;
        } else if (st == 1) {    /* optional file name */
            if (t->id != LEX_SCON || *t->spell != '"') {    /* first char; no LEX_SPELL() */
                err_dpos(t->pos, ERR_PP_ILLFNAME, LEX_SPELL(t));
                return t;
            }
            fn = recstr(t);
            st = 2;
        } else {    /* extra tokens */
            t = xtratok(t);
            break;
        }
    }

    if (st == 0)
        err_dpos(t->pos, ERR_PP_NOLINENO);
    else {
        pos = lmap_ninfo(pos->from);
        fn = (fn)? hash_new(fn+1, strlen(fn+1)-1): pos->u.i.f;    /* cis */
        lmap_from = lmap_line(fn, n-in_py, pos);
        lst_assert();
    }

    return t;
}


/*
 *  handles the "directive" state after the "normal" state
 */
static int direci(lex_t *t)
{
    int i = NELEM(dtab);

    NEXTSP(t);    /* consumes # */
    if (t->id == LEX_ID) {
        const char *n = LEX_SPELL(t);
        for (i = 0; i < NELEM(dtab); i++)
            if (strcmp(n, dtab[i].name) == 0)
                break;
        switch(i) {
            case DINCLUDE:
                t = dinclude();
                break;
            case DDEFINE:
                t = mcr_define(0);
                break;
            case DUNDEF:
                t = dundef();
                break;
            case DIF:
                break;
            case DIFDEF:
                break;
            case DIFNDEF:
                break;
            case DELIF:
                break;
            case DELSE:
                break;
            case DENDIF:
                break;
            case DLINE:
                t = dline(t->pos);
                break;
            case DERROR:
                break;
            case DPRAGMA:
                break;
            default:
                err_dpos(t->pos, ERR_PP_UNKNOWNDIR);
                break;
        }
    } else
        i = DUNDEF;

    if (warnxtra[i]) {
        SKIPSP(t);
        if (t->id != LEX_NEWLINE)
            t = xtratok(t);
    }
    SKIPNL(t);
    lst_discard(0, 1);
    lex_direc = 0;
    state = SAFTRNL;

    return 0;
}


/*
 *  handles the "directive" state after the "ignore" state
 */
static int direce(lex_t *t)
{
    while (t->id != LEX_NEWLINE) {
        assert(t->id != LEX_EOI);
        t = lst_nexti();
    }

    lst_discard(0, 1);
    lex_direc = 0;
    state = SAFTRNL;

    return 0;
}


/*
 *  preprocesses input tokens to form the output list
 */
void (proc_prep)(void)
{
    lex_t *t = lst_nexti();

    while (1) {
        switch(state) {
            case SINIT:
            case SAFTRNL:
                while (1) {
                    SKIPSP(t);
                    switch(t->id) {
                        case LEX_NEWLINE:
                            lst_flush(0, 1);
                            t = lst_nexti();
                            continue;
                        case LEX_SHARP:
                            state++;
                            assert(state == SIDIREC || state == SDIREC);
                            lex_direc = 1;
                            goto loop;
                        default:
                            state = SNORM;
                            goto loop;
                        case LEX_EOI:
                            if (inc_isffile()) {
                                lst_flush(0, 1);
                                return;
                            }
                            lst_discard(0, 1);    /* discards EOI */
                            in_switch(NULL);    /* pop */
                            t = lst_nexti();
                            goto loop;
                    }
                }
                /* assert(!"impossible control flow - should never reach here");
                   break; */
            case SIDIREC:
            case SDIREC:
                directive(t);
                t = lst_nexti();
                break;
            case SNORM:
                while (t->id != LEX_NEWLINE) {
                    assert(t->id != LEX_EOI);
                    if (t->id == LEX_ID && !t->f.blue)
                        mcr_expand(t);
                    t = lst_nexti();
                }
                lst_flush(0, 1);
                state = SAFTRNL;
                return;    /* at least newline flushed */
            case SIGN:
                // t = ign(t);
                break;
            default:
                assert(!"invalid state -- should never reach here");
                break;
        }
        loop:
            ;
    }
}

/* end of proc.c */
