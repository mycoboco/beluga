/*
 *  assertion (cbl)
 */

#ifndef ASSERT_H
#define ASSERT_H

#if defined(NDEBUG) || defined(ASSERT_STDC_VER)    /* standard version requested */

#include <assert.h>

#else    /* use "assert.h" supporting exception */

#include "cbl/except.h"    /* EXCEPT_RAISE */


/* replaces standard assert() */
#define assert(e) ((void)((e) || (EXCEPT_RAISE(assert_exceptfail), 0)))


/* exception for assertion failure */
extern const except_t assert_exceptfail;


#endif    /* NDEBUG || ASSERT_STDC_VER */

#endif    /* ASSERT_H */

/* end of assert.h */
