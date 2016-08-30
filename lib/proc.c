/*
 *  processing for preprocessor
 */

#include <cbl/assert.h>    /* assert */

#include "lex.h"
#include "lst.h"


/* processing states */
enum {
    SINIT,      /* initial or after macro guard */
    SIDIREC,    /* directive after initial */
    SAFTRNL,    /* after newline */
    SDIREC,     /* directive */
    SNORM,      /* normal */
    SIGN        /* ignore */
};


/* internal functions referenced forwardly */
static int direci(lex_t *);


static int (*directive)(lex_t *) = direci;    /* handles "directive" state */

static int state = SINIT;    /* current state */


/*
 *  handles the "directive" state after the "normal" state
 */
static int direci(lex_t *t)
{
    while (t->id != LEX_NEWLINE && t->id != LEX_EOI)
        t = lst_nexti();

    lst_discard();
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

    lst_discard();
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
                    while (t->id == LEX_SPACE)
                        t = lst_nexti();
                    switch(t->id) {
                        case LEX_NEWLINE:
                            lst_flush();
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
                            lst_flush();
                            return;
                    }
                }
                break;
            case SIDIREC:
            case SDIREC:
                directive(t);
                t = lst_nexti();
                break;
            case SNORM:
                while (t->id != LEX_EOI) {
                    if (t->id == LEX_ID) {
                    } else if (t->id == LEX_NEWLINE) {
                        lst_flush();
                        state = SAFTRNL;
                        return;    /* at least newline flushed */
                    }
                    t = lst_nexti();
                }
                lst_flush();
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
