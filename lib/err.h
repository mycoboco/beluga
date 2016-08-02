/*
 *  error handling
 */

#ifndef ERR_H
#define ERR_H

#include <cbl/except.h>    /* except_t */

#include "lmap.h"


/* error codes */
enum {
#define xx(a, b, c, d) ERR_##a,
#define yy(a, b, c, d) ERR_##a,
#include "xerror.h"
    ERR_LAST
};


extern int err_lim;                   /* # of allowed errors before stop */
extern const except_t err_except;     /* exception for too many errors */


int err_count(void);
void err_nowarn(int, int);
void err_issuel(const char *, int, int, ...);
void err_issue(const lmap_t *, int, ...);


#endif    /* ERR_H */

/* end of err.h */
