/*
 *  limits.h for x86-linux
 */

#ifndef _LIMITS
#define _LIMITS

/* signed char properties */
#define SCHAR_MAX __SCHAR_MAX__
#define SCHAR_MIN (-SCHAR_MAX-1)

/* char properties */
#define CHAR_BIT __CHAR_BIT__
#if __CHAR_UNSIGNED__
    #define CHAR_MAX UCHAR_MAX
    #define CHAR_MIN 0
#else    /* !__CHAR_UNSIGNED__ */
    #define CHAR_MAX SCHAR_MAX
    #define CHAR_MIN SCHAR_MIN
#endif    /* __CHAR_UNSIGNED__ */

/* short properties */
#define SHRT_MAX __SHRT_MAX__
#define SHRT_MIN (-SHRT_MAX-1)

/* int properties */
#define INT_MAX __INT_MAX__
#define INT_MIN (-INT_MAX-1)

/* long properties */
#define LONG_MAX __LONG_MAX__
#define LONG_MIN (-LONG_MAX-1)

#ifdef __LONG_LONG_MAX__
/* long long properties */
#define LLONG_MAX __LONG_LONG_MAX__
#define LLONG_MIN (-LLONG_MAX-1)
#endif    /* __LONG_LONG_MAX__ */

/* unsigned properties */
#define UCHAR_MAX  (SCHAR_MAX*2 + 1)
#define USHRT_MAX  (SHRT_MAX*2 + 1)
#define UINT_MAX   (INT_MAX*2U + 1)
#define ULONG_MAX  (LONG_MAX*2UL + 1)
#ifdef __LONG_LONG_MAX__
#define ULLONG_MAX (LLONG_MAX*2ULL + 1)
#endif    /* __LONG_LONG_MAX__ */

/* multibyte properties */
#define MB_LEN_MAX 16

#endif    /* _LIMITS */
