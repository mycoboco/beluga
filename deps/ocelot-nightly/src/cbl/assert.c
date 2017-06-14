/*
 *  assertion (cbl)
 */

#include "cbl/assert.h"
#include "cbl/except.h"    /* except_t */


/* exception for assertion failure */
const except_t assert_exceptfail = { "Assertion failed" };

/* end of assert.c */
