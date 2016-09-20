/*
 *  processing for preprocessor
 */

#include <string.h>        /* strcmp */
#include <cbl/arena.h>     /* ARENA_FREE */
#include <cbl/assert.h>    /* assert */

#include "common.h"
#include "err.h"
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
 *  handles the "directive" state after the "normal" state
 */
static int direci(lex_t *t)
{
    int i;

    NEXTSP(t);    /* consumes # */
    if (t->id == LEX_ID) {
        if (snlen(t->spell, 8) < 8) {
            for (i = 0; i < NELEM(dtab); i++)
                if (strcmp(t->spell, dtab[i].name) == 0)
                    break;
            switch(i) {
                case DINCLUDE:
                    break;
                case DDEFINE:
                    t = mcr_define(0);
                    break;
                case DUNDEF:
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
            err_dpos(t->pos, ERR_PP_UNKNOWNDIR);
    } else
        i = DUNDEF;

    SKIPSP(t);
    if (t->id != LEX_NEWLINE && warnxtra[i]) {
        lmap_t *s, *e;
        s = e = t->pos;
        do {
            t = lst_nexti();
            if (t->id != LEX_SPACE && t->id != LEX_NEWLINE)
                e = t->pos;
        } while(t->id != LEX_NEWLINE);
        err_dpos(lmap_range(s, e), ERR_PP_EXTRATOKEN);
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
    while (t->id != LEX_NEWLINE && t->id != LEX_EOI)
        t = lst_nexti();

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
                ARENA_FREE(strg_line);
                while (1) {
                    while (t->id == LEX_SPACE)
                        t = lst_nexti();
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
                            lst_flush(0, 1);
                            return;
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
                while (t->id != LEX_EOI) {
                    if (t->id == LEX_ID && !t->f.blue) {
                        mcr_expand(t);
                    } else if (t->id == LEX_NEWLINE) {
                        lst_flush(0, 1);
                        state = SAFTRNL;
                        return;    /* at least newline flushed */
                    }
                    t = lst_nexti();
                }
                lst_flush(0, 1);
                return;
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
