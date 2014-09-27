/*
 *  common definitions
 */

#ifndef COMMON_H
#define COMMON_H

#include <ctype.h>     /* isdigit */
#include <limits.h>    /* CHAR_BIT, ULONG_MAX */
#ifdef HAVE_ICONV
#include <errno.h>         /* errno, E2BIG */
#include <stddef.h>        /* size_t, NULL */
#include <cbl/memory.h>    /* MEM_RESIZE */
#include <iconv.h>         /* iconv */
#endif    /* HAVE_ICONV */


#include "main.h"
#ifdef SEA_CANARY
#include "../cpp/lex.h"
#endif    /* SEA_CANARY */


#ifndef SEA_CANARY
/* pint_t used in sym.h through ty.h */

/* contains pointers (including func pointers) on the target;
   ASSUMPTION: an integer type can represent pointers on the target */
typedef unsigned long pint_t;


#include "ty.h"
#ifdef HAVE_ICONV
#include "lex.h"
#endif    /* HAVE_ICONV */
#endif    /* !SEA_CANARY */

/*
 *  common macros
 */

/* suppresses warning for unused identifiers */
#define UNUSED(id) ((void)(id))

/* number of elements in array */
#define NELEM(a) ((unsigned long)(sizeof(a)/sizeof(*(a))))

/* rounds up x to */
#define ROUNDUP(x, n) (((x)+((n)-1)) & (~((n)-1)))

/* max function */
#define MAX(x, y) (((x) > (y))? (x): (y))

/* format string for printing arbitrary characters;
   "\xHH" is confusing due to "\xFFAB" output for "\xFF" "AB" */
#define ARBCHAR "<%02X>"

#define BUFUNIT 128    /* buffer resize unit */

/* constructs n 1s;
   ASSUMPTION: long on the host can represent all signed integers on the target;
   ASSUMPTION: unsigned long on the host can represent all unsigned integers on the target */
#define ONES(n) (((n) < sizeof(long)*CHAR_BIT)? ~(~0UL << (n)): ~0UL)

#ifndef SEA_CANARY
/* checks if a name/symbol is generated */
#define GENNAME(name) (isdigit(*(unsigned char *)(name)))
#define GENSYM(sym)   GENNAME((sym)->name)
#endif    /* !SEA_CANARY */

#ifdef HAVE_ICONV
/* checks if first byte of UTF-8;
   ASSUMPTION: (HAVE_ICONV) UTF-8 used as internal pivot encoding */
#define FIRSTUTF8(c) (*(unsigned char *)&(c) >> 6 != 0x02)

/* declares variables for iconv */
#define ICONV_DECL(ibufv_i, ilenv_i)    \
    char *ibufv = ibufv_i;              \
    size_t ilenv = ilenv_i;             \
    char *obufv, *obuf;                 \
    size_t olenv, olen

/* performs a conversion */
#define ICONV_DO(cd, init, handle)                                          \
    if (init)                                                               \
        iconv(*(cd), NULL, NULL, NULL, NULL);                               \
    errno = 0;                                                              \
    while (iconv(*(cd), &ibufv, &ilenv, &obufv, &olenv) == (size_t)-1) {    \
        if (errno == E2BIG) {                                               \
            MEM_RESIZE(obuf, olen+BUFUNIT);                                 \
            obufv = obuf + (olen - olenv);                                  \
            olen += BUFUNIT;                                                \
            olenv += BUFUNIT;                                               \
        } else {                                                            \
            lex_pos_t pos;                                                  \
            handle                                                          \
            break;                                                          \
        }                                                                   \
    }

/* inserts an initial sequence */
#define ICONV_INIT(cd)                                                  \
    errno = 0;                                                          \
    while (iconv(*(cd), NULL, NULL, &obufv, &olenv) == (size_t)-1) {    \
        if (errno == E2BIG) {                                           \
            MEM_RESIZE(obuf, olen+BUFUNIT);                             \
            obufv = obuf + (olen - olenv);                              \
            olen += BUFUNIT;                                            \
            olenv += BUFUNIT;                                           \
        } else                                                          \
            break;                                                      \
    }
#endif    /* HAVE_ICONV */

/* character categories */
#define ISCH_I(c)   (main_ch[(unsigned char)(c)] & 0x01)    /* isalnum || _ */
#define ISCH_IN(c)  (main_ch[(unsigned char)(c)] & 0x02)    /* isalnum || _ || \n */
#define ISCH_IP(c)  (main_ch[(unsigned char)(c)] & 0x04)    /* isalnum || _ || . */
#define ISCH_APN(c) (main_ch[(unsigned char)(c)] & 0x08)    /* isalnum || . || \n */
#define ISCH_DN(c)  (main_ch[(unsigned char)(c)] & 0x10)    /* isdigit || \n */
#define ISCH_XN(c)  (main_ch[(unsigned char)(c)] & 0x20)    /* isxdigit || \n */

/* true if host uses little endian;
   need to declare endian as static int and initialize to 1;
   ASSUMPTION: the host uses either little or big endian */
#define LITTLE (((unsigned char *)(&endian))[0] == 1)

/* changes endianness of an object */
#define CHGENDIAN(x, s)                                                       \
    do {                                                                      \
        int i;                                                                \
        unsigned char t;                                                      \
        for (i = 0; i < (s) / 2; i++)                                         \
            t = ((unsigned char *)&(x))[i],                                   \
            ((unsigned char *)&(x))[i] = ((unsigned char *)&(x))[(s)-1-i],    \
            ((unsigned char *)&(x))[(s)-1-i] = t;                             \
    } while(0)


/*
 *  translation limits
 */

#ifndef SEA_CANARY
/* nesting levels of blocks */
#define TL_BLOCK_STD (main_tl()->block)
#define TL_BLOCK_C90 15                    /* C90 5.2.4.1 */
#define TL_BLOCK_C99 127                   /* C99 5.2.4.1 */

/* pointer, array and function modifications in declarations */
#define TL_DECL_STD (main_tl()->decl)
#define TL_DECL_C90 12                   /* C90 5.2.4.1 */
#define TL_DECL_C99 12                   /* C99 5.2.4.1 */

/* nesting levels of parenthesized declarators */
#define TL_PAREND_STD (main_tl()->parend)
#define TL_PAREND_C90 31                     /* C90 5.2.4.1 */
#define TL_PAREND_C99 63                     /* C99 5.2.4.1 */

/* significant initial characters in external names */
#define TL_ENAME_STD (main_tl()->ename)
#define TL_ENAME_C90 6     /* C90 5.2.4.1 */
#define TL_ENAME_C99 63    /* C99 5.2.4.1 */

/* external identifiers in one translation unit */
#define TL_NAME_STD (main_tl()->name)
#define TL_NAME_C90 511                  /* C90 5.2.4.1 */
#define TL_NAME_C99 4095                 /* C99 5.2.4.1 */

/* identifiers with block scope in one block */
#define TL_NAMEB_STD (main_tl()->nameb)
#define TL_NAMEB_C90 127                   /* C90 5.2.4.1 */
#define TL_NAMEB_C99 511                   /* C99 5.2.4.1 */

/* parameters in one function definition */
#define TL_PARAM_STD (main_tl()->param)
#define TL_PARAM_C90 31                    /* C90 5.2.4.1 */
#define TL_PARAM_C99 127                   /* C99 5.2.4.1 */

/* arguments in one function call */
#define TL_ARG_STD (main_tl()->arg)
#define TL_ARG_C90 31                  /* C90 5.2.4.1 */
#define TL_ARG_C99 127                 /* C99 5.2.4.1 */

/* string literal */
#define TL_STR     4095
#define TL_STR_STD (main_tl()->str)
#define TL_STR_C90 509                 /* C90 5.2.4.1 */
#define TL_STR_C99 4095                /* C99 5.2.4.1 */

/* bytes in an object (hosted only) */
#define TL_OBJ_STD (main_tl()->obj)
#define TL_OBJ_C90 32767UL             /* C90 5.2.4.1 */
#define TL_OBJ_C99 65535UL             /* C99 5.2.4.1 */

/* case labels for a switch (excluding nested ones) */
#define TL_NCASE_STD (main_tl()->ncase)
#define TL_NCASE_C90 257                   /* C90 5.2.4.1 */
#define TL_NCASE_C99 1023                  /* C99 5.2.4.1 */

/* members in a single structure or union */
#define TL_MBR_STD (main_tl()->mbr)
#define TL_MBR_C90 127                 /* C90 5.2.4.1 */
#define TL_MBR_C99 1023                /* C99 5.2.4.1 */

/* enumeration constants in an enumeration */
#define TL_ENUMC_STD (main_tl()->enumc)
#define TL_ENUMC_C90 127                   /* C90 5.2.4.1 */
#define TL_ENUMC_C99 1023                  /* C99 5.2.4.1 */

/* leves of nested structure or union definitions */
#define TL_STRCT_STD (main_tl()->strct)
#define TL_STRCT_C90 15                    /* C90 5.2.4.1 */
#define TL_STRCT_C99 63                    /* C99 5.2.4.1 */
#endif    /* !SEA_CANARY */

/* significant initial characters in internal or (pp) macro names */
#define TL_INAME_STD (main_tl()->iname)
#define TL_INAME_C90 31                    /* C90 5.2.4.1 */
#define TL_INAME_C99 63                    /* C99 5.2.4.1 */

/* nesting levels of parenthesized expressions */
#define TL_PARENE_STD (main_tl()->parene)
#define TL_PARENE_C90 32                     /* C90 5.2.4.1 */
#define TL_PARENE_C99 63                     /* C99 5.2.4.1 */

/* logical source line */
#define TL_LINE     4095
#define TL_LINE_STD (main_tl()->line)
#define TL_LINE_C90 509                  /* C90 5.2.4.1 */
#define TL_LINE_C99 4095                 /* C99 5.2.4.1 */

/* line number */
#define TL_LINENO_STD (main_tl()->lineno)
#define TL_LINENO_C90 ULONG_MAX              /* unspecified */
#define TL_LINENO_C99 2147483647UL           /* C99 6.10.4 */

/* (pp) nesting levels for #included files */
#define TL_INC     256
#define TL_INC_STD (main_tl()->inc)
#define TL_INC_C90 8                   /* C90 5.2.4.1 */
#define TL_INC_C99 15                  /* C99 5.2.4.1 */

#ifdef SEA_CANARY
/* (pp) nesting levels of conditional inclusion */
#define TL_COND_STD (main_tl()->cond)
#define TL_COND_C90 8                    /* C90 5.2.4.1 */
#define TL_COND_C99 63                   /* C99 5.2.4.1 */

/* (pp) macros simultaneously defined in one pp-translation unit */
#define TL_PPNAME_STD (main_tl()->ppname)
#define TL_PPNAME_C90 1024                   /* C90 5.2.4.1 */
#define TL_PPNAME_C99 4095                   /* C99 5.2.4.1 */

/* (pp) parameters in one macro definition */
#define TL_PARAMP_STD (main_tl()->paramp)
#define TL_PARAMP_C90 31                     /* C90 5.2.4.1 */
#define TL_PARAMP_C99 127                    /* C99 5.2.4.1 */

/* (pp) arguments in one macro invocation */
#define TL_ARGP_STD (main_tl()->argp)
#define TL_ARGP_C90 31                   /* C90 5.2.4.1 */
#define TL_ARGP_C99 127                  /* C99 5.2.4.1 */

/* (pp) */
#define TL_VER_STD (main_tl()->ver)
#define TL_VER_C90 "199409L"
#define TL_VER_C99 "199901L"
#endif    /* SEA_CANARY */


/*
 *  target parameters
 */

/* bits in a byte;
   best effort to distinguish the host's byte from the target's
   but would not work when TG_CHARBIT != CHAR_BIT */
#ifndef TG_CHAR_BIT
#define TG_CHAR_BIT CHAR_BIT
#endif    /* TG_CHAR_BIT */

#ifndef SEA_CANARY
/* smallest/largest values of signed char */
#define TG_SCHAR_MIN (ty_schartype->u.sym->u.lim.min.li)
#define TG_SCHAR_MAX (ty_schartype->u.sym->u.lim.max.li)

/* largest value of unsigned char */
#define TG_UCHAR_MAX (ty_uchartype->u.sym->u.lim.max.ul)

/* smallest/largest values of short */
#define TG_SHRT_MIN (ty_shorttype->u.sym->u.lim.min.li)
#define TG_SHRT_MAX (ty_shorttype->u.sym->u.lim.max.li)

/* largest value of unsigned short */
#define TG_USHRT_MAX (ty_ushorttype->u.sym->u.lim.max.ul)

/* smallest/largest values of int */
#define TG_INT_MIN (ty_inttype->u.sym->u.lim.min.li)
#define TG_INT_MAX (ty_inttype->u.sym->u.lim.max.li)

/* largest value of unsigned int */
#define TG_UINT_MAX (ty_unsignedtype->u.sym->u.lim.max.ul)

/* smallest/largest values of long */
#define TG_LONG_MIN (ty_longtype->u.sym->u.lim.min.li)
#define TG_LONG_MAX (ty_longtype->u.sym->u.lim.max.li)

/* largest value of unsigned long */
#define TG_ULONG_MAX (ty_ulongtype->u.sym->u.lim.max.ul)

/* largest value of wchar_t, unsigned wchar_t and wint_t */
#define TG_WUCHAR_MAX (ty_wuchartype->u.sym->u.lim.max.ul)
#define TG_WCHAR_MIN  (ty_wchartype->u.sym->u.lim.min.li)
#define TG_WCHAR_MAX  (ty_wchartype->u.sym->u.lim.max.ul)
#define TG_WINT_MIN   (ty_winttype->u.sym->u.lim.min.li)
#define TG_WINT_MAX   (ty_winttype->u.sym->u.lim.max.li)

/* smallest/largest values of float */
#define TG_FLT_MIN (ty_floattype->u.sym->u.lim.min.f)
#define TG_FLT_MAX (ty_floattype->u.sym->u.lim.max.f)

/* smallest/largest values of double */
#define TG_DBL_MIN (ty_doubletype->u.sym->u.lim.min.d)
#define TG_DBL_MAX (ty_doubletype->u.sym->u.lim.max.d)

/* smallest/largest value of long double */
#define TG_LDBL_MIN (ty_ldoubletype->u.sym->u.lim.min.ld)
#define TG_LDBL_MAX (ty_ldoubletype->u.sym->u.lim.max.ld)

/* NaNs */
#define TG_FLT_NAN  (ty_floattype->u.sym->u.lim.max.f)
#define TG_DBL_NAN  (ty_doubletype->u.sym->u.lim.max.d)
#define TG_LDBL_NAN (ty_ldoubletype->u.sym->u.lim.max.ld)
#endif    /* !SEA_CANARY */


#endif    /* COMMON_H */

/* end of common.h */
