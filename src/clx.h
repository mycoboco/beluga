/*
 *  compiler lexical analyzer
 */

#ifndef CLX_H
#define CLX_H

#include "lex.h"
#include "lmap.h"
#include "lst.h"
#include "sym.h"


void clx_init(void);
ux_t clx_ccon(lex_t *, int *);
int clx_next(void);
int clx_xtracomma(int, const char *, int);
int clx_tyla(const char *);


extern int clx_tc;                /* token code of current token */
extern const char *clx_tok;       /* token spelling of current token */
extern sym_t *clx_sym;            /* symbol table entry for current token */
extern const lmap_t *clx_cpos;    /* locus of current token */
extern const lmap_t *clx_ppos;    /* locus of previous token */

extern const char * const clx_name[];    /* token names */
extern const char clx_kind[];            /* token kinds */


#define clx_peek() (lst_peekns()->id)    /* looks ahead next token */

/* checks if token denotes type */
#define clx_istype(s) (clx_kind[clx_tc] == LEX_CHAR ||                                         \
                       (clx_tc == LEX_ID && ((clx_sym && clx_sym->sclass == LEX_TYPEDEF) ||    \
                                             (!clx_sym && clx_tyla(s)))))

/* macros to check current token kind */
#define clx_isadcl()   (clx_tc == LEX_ID || clx_tc == '*' || clx_tc == '(' || clx_tc == '[')
#define clx_isdcl()    (clx_tc == LEX_ID || clx_tc == '*' || clx_tc == '(')
#define clx_ispdcl()   (clx_tc == '*' || clx_tc == '(' || clx_tc == '[')
#define clx_issdecl(s) (clx_kind[clx_tc] == LEX_STATIC || clx_istype(s))             /* start */
#define clx_ispdecl()  (clx_issdecl(CLX_TYLA) && clx_peek() != ':')                  /* pure */
#define clx_isparam(a) (clx_issdecl(CLX_TYLAP a))
#define clx_isexpr()   (clx_kind[clx_tc] == LEX_ID)
#define clx_issstmt()  (clx_kind[clx_tc] == LEX_ID || clx_kind[clx_tc] == LEX_IF)    /* start */
#define clx_ispstmt()  (clx_issstmt() &&    \
                        (!clx_istype(CLX_TYLA) || clx_peek() == ':'))                /* pure */

/* type look-ahead token sets */
#define CLX_TYLA  "\x1E*"     /* default; id, * */
#define CLX_TYLAF "\x1E*;"    /* struct/union members; id, *, ; */
#define CLX_TYLAP CLX_TYLA    /* prototype parameters */
#define CLX_TYLAN NULL        /* non-prototype parameters */
#define CLX_TYLAS "\x1E*"     /* sizeof; id, * */
#define CLX_TYLAC "\x1E)"     /* type cast; id, ) */

/* after previous token or at current one */
#define CLX_PCPOS() ((clx_ppos)? lmap_after(clx_ppos): clx_cpos)


#endif    /* CLX_H */

/* end of clx.h */
