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


extern int err_lim;                  /* # of allowed errors before stop */
extern const except_t err_except;    /* exception for too many errors */


void err_init(void);
int err_count(void);
void err_nowarn(int, int);

void err_dpos(const lmap_t *, int, ...);
void err_dmpos(const lmap_t *, int, ...);
void err_dline(const char *, int, int, ...);


#endif    /* ERR_H */

/* end of err.h */
