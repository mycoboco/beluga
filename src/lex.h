/*
 *  lexical analyzer
 */

#ifndef LEX_H
#define LEX_H

#include <stddef.h>    /* size_t */


/* lex_pos_t and LEX_* used in sym.h */

/* locus of tokens */
typedef struct lex_pos_t {
    unsigned long c;     /* include count */
    unsigned long fy;    /* line # of first input file */
    const char *f;       /* filename */
    unsigned long y;     /* line # */
    unsigned long x;     /* character # */
    unsigned long n;     /* token length; max means not available */
} lex_pos_t;

/* token codes */
enum {
#define xx(a, b, c, d, e, f, g, h) LEX_##a = b,
#define yy(a, b, c, d, e, f, g, h)
#include "xtoken.h"
    LEX_LAST
};


#include "sym.h"


/* buffer wrapper */
typedef struct lex_buf_t {
    struct {
        unsigned char *p;
        size_t n;
    } b;    /* formerly lex_buf; buffer for recognizing strings */
    struct {
        unsigned char *p;
        size_t n;
    } s;    /* formerly lex_sbuf; source string buffer */
    struct {
        struct {
            lex_pos_t pos;        /* locus */
            unsigned long idx;    /* index in input buffer */
        } *p;
        int n;
    } t;    /* formerly lex_stop; stop positions for string */
} lex_buf_t;


extern int lex_tc;             /* token code for current token */
extern const char *lex_tok;    /* string representation of current token */
extern sym_t *lex_sym;         /* symbol table entry for current token */
extern lex_pos_t *lex_cpos;    /* locus of current token */
extern lex_pos_t *lex_ppos;    /* locus of previous token */
extern lex_buf_t lex_buf;      /* buffer set for recognizing strings */
extern lex_buf_t *lex_bp;      /* pointer to current buffer set */

extern const char * const lex_name[];    /* token names */
extern const char lex_kind[];            /* token kinds */


const lex_pos_t *lex_epos(void);
const char *lex_outpos(const lex_pos_t *);
int lex_getchr(void);
unsigned long lex_scon(int, int *, int);
int lex_next(void);
int lex_extracomma(int, const char *, int);


/* checks if token denotes type */
#define lex_istype() (lex_kind[lex_tc] == LEX_CHAR ||    \
                      (lex_tc == LEX_ID && lex_sym && lex_sym->sclass == LEX_TYPEDEF))

/* macros to check current token kind */
#define lex_isadcl()  (lex_tc == LEX_ID || lex_tc == '*' || lex_tc == '(' || lex_tc == '[')
#define lex_isdcl()   (lex_tc == LEX_ID || lex_tc == '*' || lex_tc == '(')
#define lex_issdecl() (lex_kind[lex_tc] == LEX_STATIC || lex_istype())              /* start */
#define lex_ispdecl() (lex_issdecl() && lex_getchr() != ':')                        /* pure */
#define lex_isparam() (lex_issdecl())
#define lex_isexpr()  (lex_kind[lex_tc] == LEX_ID)
#define lex_issstmt() (lex_kind[lex_tc] == LEX_ID || lex_kind[lex_tc] == LEX_IF)    /* start */
#define lex_ispstmt() (lex_issstmt() && (!lex_istype() || lex_getchr() == ':'))     /* pure */


#endif    /* LEX_H */

/* end of lex.h */
