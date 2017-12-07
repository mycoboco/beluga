/*
 *  assertion (cbl)
 */

#ifndef ASSERT_H
#define ASSERT_H

#include "cbl/except.h"    /* except_t, EXCEPT_RAISE */


#if defined(NDEBUG) || defined(ASSERT_STDC_VER)    /* standard version requested */
#include <assert.h>
#else    /* use "assert.h" supporting exception */
/* replaces standard assert() */
#define assert(e) ((void)((e) || (EXCEPT_RAISE(assert_exceptfail), 0)))
#endif    /* NDEBUG || ASSERT_STDC_VER */


/* exception for assertion failure */
extern const except_t assert_exceptfail;


#endif    /* ASSERT_H */

/* end of assert.h */
