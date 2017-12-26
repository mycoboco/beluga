/* -Wv -Dcmd */

#define foo1
foo1

#define foo2    /* unused */

#define foo3 1
#if foo3
#endif

#define foo4
#ifdef foo4
#endif

#define foo5
#ifndef foo5
#endif

#define foo6
#define foo7
#if defined foo6 && defined(foo7)
#endif

#define foo8    /* unused */
#undef foo8

#define foo9 foo    /* unused */
#define foo9 foo    /* unused */

#define foo10 foo    /* unused but error */
#define foo10 bar    /* unused */

#define foo11    /* unused */
#if 0
foo11
#endif

#define nothere1             /* used in included file */
#define nothere2             /* used in included file */
#define nothere3()           /* unused */
#define nothere4()
#define nothere5             /* unused */
#define nothere6 nothere6    /* unused */
#define nothere7             /* unused */
#include "pp-122-b.c"
#undef fromhere

#define foo12()    /* unused */
foo12

#define foo13()    /* unused */
#if foo13
#endif

#define foo14()
#ifdef foo14
#endif

#define foo15
#define exp1 foo15
exp1

#define foo16         /* unused */
#define exp2 foo16
#ifdef exp2
#endif

#define foo17 "pp-122-b.c"
#include foo17

#define foo18
#if 0
#elif defined foo18
#endif

#define foo19    /* unused */
#if 1
#elif defined foo19
#endif

#define foo20 100
#line foo20

#define foo21 foo21    /* unused */
#warning foo21

#define foo22    /* unused */
#pragma foo22
