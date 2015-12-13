/*
 *  limits.h for x86-linux
 */

#ifndef _LIMITS
#define _LIMITS

/* char properties */
#define CHAR_BIT 8
#if __CHAR_UNSIGNED__
    #define CHAR_MAX UCHAR_MAX
    #define CHAR_MIN 0
#else    /* !__CHAR_UNSIGNED__ */
    #define CHAR_MAX SCHAR_MAX
    #define CHAR_MIN SCHAR_MIN
#endif    /* __CHAR_UNSIGNED__ */

/* int properties */
#define INT_MAX  2147483647
#define INT_MIN  (-INT_MAX-1)
#define UINT_MAX 4294967295

/* long properties */
#define LONG_MAX 2147483647L
#define LONG_MIN (-LONG_MAX-1)

/* multibyte properties */
#define MB_LEN_MAX 16

/* signed char properties */
#define SCHAR_MAX 127
#define SCHAR_MIN (-SCHAR_MAX-1)

/* short properties */
#define SHRT_MAX 32767
#define SHRT_MIN (-SHRT_MAX-1)

/* unsigned properties */
#define UCHAR_MAX 255
#define ULONG_MAX 4294967295UL
#define USHRT_MAX 65535

#endif    /* _LIMITS */
