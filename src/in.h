/*
 *  input handling
 */

#ifndef IN_H
#define IN_H

#include <ctype.h>         /* isspace */
#include <stdio.h>         /* FILE */
#include <cbl/assert.h>    /* assert */

#include "common.h"
#ifndef SEA_CANARY
#include "err.h"
#endif    /* !SEA_CANARY */


/* element for #include list */
typedef struct in_inc_t {
    const char *f;          /* file that #included current file  */
    unsigned long y;        /* line number of #include */
    unsigned printed: 1;    /* true if already printed */
} in_inc_t;

/* locus for input files */
typedef struct in_pos_t {
    unsigned long c;     /* include count */
    const char *ff;      /* name of first input file */
    unsigned long fy;    /* line number for first input file */
    const char *f;       /* name of current input file */
    unsigned long y;     /* line number of current line */
    unsigned n: 1;       /* true if current file is first */
#ifdef SEA_CANARY
    const char *mf;      /* file name by #line */
    unsigned long my;    /* line number by #line */
#endif    /* SEA_CANARY */
} in_pos_t;


extern in_pos_t in_cpos;                 /* current locus for input file */
extern const unsigned char *in_line;     /* beginning of current line */
extern const unsigned char *in_cp;       /* current character */
extern const unsigned char *in_limit;    /* end of current input buffer */
extern unsigned long in_outlen;          /* length of deleted part (0 when usedynamic) */
extern in_inc_t **in_incp;               /* #include list */
extern void (*in_nextline)(void);        /* function to read next input line */


void in_enterfunc(void);
void in_exitfunc(void);
const unsigned char *in_getline(unsigned long, const char *, unsigned long);
unsigned long in_cntchar(const unsigned char *, const unsigned char *);
#ifdef SEA_CANARY
void in_switch(FILE *, const char *);
void in_toperm(void);
#endif    /* SEA_CANARY */
void in_init(FILE *, const char *);
void in_fillbuf(void);
void in_close(void);


#define IN_MAXTOKEN 32             /* max length of common tokens */
#define IN_MAXLINE  TL_LINE        /* (!usedynamic) max length of unconsumed tail */

/* treats newline; do-while(0) cannot be used */
#define IN_FILLBREAK(p)                 \
            {                           \
                if ((p) < in_limit)     \
                    break;              \
                in_cp = (p);            \
                in_fillbuf();           \
                (p) = in_cp;            \
                if ((p) == in_limit)    \
                    break;              \
                continue;               \
            }

#ifndef SEA_CANARY
/* skips spaces; stops at non-space char in current line */
#define IN_SKIPSP(p)                                                       \
            do {                                                           \
                while (isspace(*(p))) {                                    \
                    if (*(p) == '\n') {                                    \
                        IN_FILLBREAK(p) /* ; */                            \
                    } else if (*(p) != ' ' && *(p) != '\t') {              \
                        assert(*(p)=='\v' || *(p)=='\f' || *(p)=='\r');    \
                        in_cp = (p);                                       \
                        err_issue(ERR_LEX_STRAYWS,                         \
                                  (*(p) == '\v')? "\\v":                   \
                                  (*(p) == '\f')? "\\f": "\\r");           \
                    }                                                      \
                    (p)++;                                                 \
                }                                                          \
            } while(0)
#endif    /* SEA_CANARY */

/* discards current line */
#define IN_DISCARD(p)                       \
            do {                            \
                if (*(p)++ == '\n') {       \
                    if ((p) <= in_limit)    \
                        break;              \
                    in_cp = (p);            \
                    in_fillbuf();           \
                    (p) = in_cp;            \
                    if ((p) == in_limit)    \
                        break;              \
                }                           \
            } while(1)


#endif    /* IN_H */

/* end of in.h */
