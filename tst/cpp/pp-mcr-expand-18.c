/* -Wv */

#ifdef __FILE__
Okay
#else
No
#endif

#ifdef __LINE__
Okay
#endif

#define FOO(x) x - x
#if FOO(__LINE__)
#else
Okay
#endif

#if FOO(__FILE__)
#endif
