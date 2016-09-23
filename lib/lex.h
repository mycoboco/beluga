/*
 *  primitive lexical analyzer
 */

#ifndef LEX_H
#define LEX_H

#include <cbl/arena.h>    /* arena_t */

#include "common.h"
#include "lmap.h"


/* token */
typedef struct lex_t {
    short id;             /* token code */
    const char *spell;    /* text spelling */
    const lmap_t *pos;          /* token locus */
    struct {
        unsigned alloc: 1;    /* true if buffer allocated for spelling */
        unsigned clean: 1;    /* true if no line splicing or trigraphs */
        unsigned end:   1;    /* true if LEX_MCR denotes end of expansion */
        unsigned blue:  1;    /* true if painted blue */
    } f;
    struct lex_t *next;    /* link for token list */
} lex_t;

/* token codes */
enum {
#define xx(a, b, c, d, e, f, g, h) LEX_##a = b,
#define yy(a, b, c, d, e, f, g, h)
#include "xtoken.h"
    LEX_LAST
};


extern int lex_inc;        /* true while parsing #include */
extern int lex_direc;      /* true while parsing directives */
extern int lex_fromstr;    /* true while input coming from string */


lex_t *lex_next(void);
lex_t *lex_make(int, const char *, int, arena_t *);
const char *lex_spell(const lex_t *);


/* gets "clean" spelling of token */
#define LEX_SPELL(t) (((t)->f.clean)? (t)->spell: lex_spell(t))

/* deallocates token for ordinary cases */
#define LEX_FREE(t)                            \
    do {                                       \
        if ((t)->f.alloc) {                    \
            void *tmp = (void *)(t)->spell;    \
            MEM_FREE(tmp);                     \
        }                                      \
        MEM_FREE(t);                           \
    } while(0)


#endif    /* LEX_H */

/* end of lex.h */
