/*
 *  lexical analyzer for preprocessor
 */

#ifndef LEX_H
#define LEX_H

#include "../src/alist.h"


/* locus of tokens */
typedef struct lex_pos_t {
    unsigned long c;     /* include count */
    unsigned long fy;    /* line # of first input file */
    const char *f;       /* filename */
    unsigned long y;     /* line # */
    unsigned long x;     /* character # */
} lex_pos_t;

/* represents a token */
typedef struct lex_t {
    short id;              /* token code */
    unsigned char blue;    /* carries blue flag; see lxl_tolxl();
                              should be bit-field if other flags added */
    const char *rep;       /* text representation */
} lex_t;

/* token codes */
enum {
#define xx(a, b, c, d, e, f, g, h) LEX_##a = b,
#define yy(a, b, c, d, e, f, g, h)
#include "../src/xtoken.h"
    LEX_LAST
};


extern int lex_inc;           /* true while parsing #include */
extern int lex_direc;         /* true while parsing directives */
extern lex_pos_t lex_cpos;    /* locus of current token */


lex_t *lex_nexttok(void);
alist_t *lex_run(const char *, const lex_pos_t *);
unsigned long lex_bs(const char **, unsigned long, const lex_pos_t *, const char *);
const char *lex_outpos(const lex_pos_t *);


#endif    /* LEX_H */

/* end of lex.h */
