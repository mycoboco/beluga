/*
 *  stdarg.h for x86-linux
 */

#ifndef _STDARG
#define _STDARG


typedef char *va_list;

#ifndef _CHAR
#define _CHAR
typedef char __char;
#endif    /* _CHAR */

#ifndef _VOID
#define _VOID
typedef void __void;
#endif    /* _VOID */

#define _Align       4U    /* 4-byte alignment */
#define _Rndup(x, n) ((sizeof(x)+((n)-1)) & (~((n)-1)))    /* use of sizeof: non-conforming */

#define va_arg(ap, T)   (*(T *)(((ap) += _Rndup(T, _Align)) - _Rndup(T, _Align)))
#define va_end(ap)      ((__void)0)
#define va_start(ap, L) (__void)((ap) = (__char *)&(L) + _Rndup(L, _Align))

#endif    /* _STDARG */

/* end of stdarg.h */
