/*
 *  error handling
 */

#ifndef ERR_H
#define ERR_H

#include <cbl/except.h>    /* except_t */

#include "lmap.h"
#include "sym.h"


/* error codes */
enum {
#define xx(a, b, c, d) ERR_##a,
#define yy(a, b, c, d) ERR_##a,
#include "xerror.h"
    ERR_LAST
};


extern int err_mute;                 /* true if diagnostics suppressed */
extern int err_lim;                  /* # of allowed errors before stop */
extern const except_t err_except;    /* exception for too many errors */


void err_init(void);
int err_count(void);
void err_nowarn(int, int);
int err_experr(void);
void err_cleareff(void);

int err_dpos(const lmap_t *, int, ...);
int err_dmpos(const lmap_t *, int, ...);
int err_dline(const char *, int, int, ...);

const sym_t *err_idsym(const char *);


/* turns off diagnostics except fatal ones in a nestable way */
#define err_mute()   ((void)err_mute++)
#define err_unmute() (err_mute--, assert(err_mute >= 0))


#endif    /* ERR_H */

/* end of err.h */
