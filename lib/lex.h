/*
 *  primitive lexical analyzer
 */

#ifndef LEX_H
#define LEX_H


/* used in lmap.h;
   lmap_spell() makes mutual references between lex.h and lmap.h */
typedef struct lex_t lex_t;


#include "common.h"
#include "lmap.h"


/* token */
struct lex_t {
    short id;             /* token code */
    const char *spell;    /* text spelling */
    const lmap_t *pos;    /* token locus */
    struct {
        unsigned alloc: 1;    /* true if buffer allocated for spelling */
        unsigned clean: 1;    /* true if no line splicing or trigraphs */
        unsigned end:   1;    /* true if LEX_MCR denotes end of expansion */
        unsigned blue:  1;    /* true if painted blue */
        unsigned noarg: 1;    /* true if token came from ## */
        unsigned sync:  2;    /* 1: #include start, 2: end */
        unsigned vaarg: 1;    /* true if __VA_ARGS__ diagnosed */
    } f;
    struct lex_t *next;    /* link for token list */
};

/* token codes */
enum {
#define xx(a, b, c, d, e, f, g, h) LEX_##a = b,
#define kk(a, b, c, d, e, f, g, h) LEX_##a = b,
#define yy(a, b, c, d, e, f, g, h)
#include "xtoken.h"
    LEX_LAST
};


extern int lex_inc;      /* true while parsing #include */
extern int lex_direc;    /* true while parsing directives */


lex_t *lex_next(void);
ux_t lex_bs(lex_t *, const char *, const char **, ux_t, const char *);
lex_t *lex_make(int, const char *, int);
const char *lex_spell(const lex_t *);
void lex_backup(int, const lmap_t *);


/* gets "clean" spelling of token */
#define LEX_SPELL(t) (((t)->f.clean)? (t)->spell: lex_spell(t))

/* backs up and restores side effects from token recognization */
#define lex_backup(pos) (lex_backup(0, (pos)))
#define lex_restore()   ((lex_backup)(1, NULL))


#endif    /* LEX_H */

/* end of lex.h */
