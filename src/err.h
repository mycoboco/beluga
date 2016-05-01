/*
 *  error handling
 */

#ifndef ERR_H
#define ERR_H

#include <cbl/except.h>    /* except_t */

#ifdef SEA_CANARY
#include "../cpp/lex.h"
#else    /* !SEA_CANARY */
#include "lex.h"
#include "sym.h"
#endif    /* !SEA_CANARY */


/* error codes */
enum {
#define xx(a, b, c, d) ERR_##a,
#define yy(a, b, c, d) ERR_##a,
#include "xerror.h"
    ERR_LAST
};


extern int err_lim;                   /* # of allowed errors before stop */
extern const except_t err_except;     /* exception for too many errors */
#ifdef HAVE_ICONV
extern char *err_cvbuf;               /* encoding conversion buffer */
#endif    /* HAVE_ICONV */

#ifndef SEA_CANARY
/* predefined stop sets to handle syntax errors */
extern const char err_sset_field[];
extern const char err_sset_strdef[];
extern const char err_sset_enumdef[];
extern const char err_sset_decl[];
extern const char err_sset_declf[];
extern const char err_sset_declb[];
extern const char err_sset_expr[];
extern const char err_sset_exprasgn[];
extern const char err_sset_initf[];
extern const char err_sset_initb[];
#endif    /* !SEA_CANARY */


void err_init(void);
int err_count(void);
#ifndef SEA_CANARY
void err_skipend(void);
void err_expect(int);
void err_skipto(int, const char []);
void err_test(int, const char []);
#endif    /* !SEA_CANARY */
void err_mute(int);
void err_unmute(int);
void err_nowarn(int, int);
#ifndef SEA_CANARY
int err_experr(void);
void err_cleareff(void);
void err_entersite(const lex_pos_t *);
const lex_pos_t *err_getppos(void);
void err_exitsite(void);
void err_issue_s(int, ...);
#endif    /* !SEA_CANARY */
void err_issuep(const lex_pos_t *, int, ...);
void err_issue(int, ...);
#ifndef SEA_CANARY
const sym_t *err_idsym(const char *);
#endif    /* !SEA_CANARY */


/* wrappers for err_(un)mute() */
#define err_mute()    ((err_mute)(0))
#define err_unmute()  ((err_unmute)(0))
#define err_wmute()   ((err_mute)(1))
#define err_wunmute() ((err_unmute)(1))


#endif    /* ERR_H */

/* end of err.h */
