__VA_ARGS__

#define __VA_ARGS__ nothing
#nondirective __VA_ARGS__    /* error */

#line 10 __VA_ARGS__

#if __VA_ARGS__
#endif

#ifdef __VA_ARGS__
#endif

#if 0
#elif __VA_ARGS__
#endif

#if 1
#elif __VA_ARGS__    /* ignored */
#endif

#if 0
#else __VA_ARGS__     /* extra */
#endif __VA_ARGS__    /* extra */

#undef __VA_ARGS__

#define __VA_ARGS__(...) #__VA_ARGS__
__VA_ARGS__(foo, bar)

#line __VA_ARGS__

#undef __VA_ARGS__

#define __VA_ARGS__ 10
#line __VA_ARGS__

#error testing __VA_ARGS__

#pragma __VA_ARGS__         /* unknown #pragma */
#pragma stdc __VA_ARGS__    /* unknown #pragma */

#if 0
__VA_ARGS__    /* ignored */
#endif

#if 1
#else
__VA_VARGS__    /* ignored */
#endif

#include __VA_ARGS__
