/*
 *  main
 */

#ifndef MAIN_H
#define MAIN_H

#include <limits.h>    /* UCHAR_MAX */

#ifdef HAVE_ICONV
#include <iconv.h>    /* iconv_t */
#endif    /* HAVE_ICONV */


/* program options;
   no bit-fields due to being used for opt */
struct main_opt {
    const char *prgname;    /* program name */
    int std;                /* 0: non-std, 1: C89/C90/C95, 2: C99;
                               resets extension and sets trigraph */
    int diagstyle;          /* 0: non-parsable & no source, 1: source line */
    int wchart;             /* 0: wchar_t is long, 1: u-short, 2: int */
    int logicshift;         /* 0: >> performs arithmetic shift, 1: logical shift */
    int uchar;              /* 0: plain char is signed, 1: unsigned */
    int extension;          /* GCC-compatible mode; resets std */
#ifdef HAVE_COLOR
    int color;              /* 0: don't colorize diagnostics, 1: do, 2: auto-detect */
#endif    /* HAVE_COLOR */
    int warncode;           /* displays warning codes if set */
    int _internal;          /* true if invoked by bcc */
#ifdef HAVE_ICONV
    const char *icset;      /* charset for source text (ASCII if not set) */
    const char *ecset;      /* charset for char/string const (ASCII if not set) */
    const char *wcset;      /* charset for wide char/string const (ASCII if not set) */
#endif    /* HAVE_ICONV */

    /* for compiler proper */
    int sizet;              /* 0: size_t is uint, 1: size_t is u-long */
    int ptrdifft;           /* 0: ptrdiff_t is int, 1: ptrdiff_t is long */
    int ptrlong;            /* 0: pointers have same size as int, 1: as long */
    int pfldunsign;         /* 0: plain bit-field is signed, 1: unsigned */
    int xref;               /* cross-reference info generated if set */
    int glevel;             /* debugging level */
    int proto;              /* prints prototype declarations for globals */
    int unwind;             /* unwind typedef names */
#ifndef NDEBUG
    int _debug;             /* (internal) prints debugging info for back-end */
#endif    /* !NDEBUG */

    /* for preprocessor */
    int pponly;             /* runs preprocessor only if set */
    int trigraph;           /* recognizes trigraphs if set (only for pp) */
    int little_endian;      /* 0: big endian on the target, 1: little, 2: same as the host */
    int stricterr;          /* 0: #error does not stop, 1: #error stops */
    int nostdinc;           /* don't follow system include paths if set */
};

/* translation limits */
struct main_tl {
    int iname;          /* TL_INAME_STD */
    int parene;         /* TL_PARENE_STD */
    sz_t line;          /* TL_LINE_STD */
    sz_t lineno;        /* TL_LINENO_STD */
    int inc;            /* TL_INC_STD */

    /* for compiler proper */
    int block;          /* TL_BLOCK_STD */
    int decl;           /* TL_DECL_STD */
    int parend;         /* TL_PAREND_STD */
    int ename;          /* TL_ENAME_STD */
    int name;           /* TL_NAME_STD */
    int nameb;          /* TL_NAMEB_STD */
    int param;          /* TL_PARAM_STD */
    int arg;            /* TL_ARG_STD */
    sz_t str;           /* TL_STR_STD */
    sz_t obj;           /* TL_OBJ_STD */
    int ncase;          /* TL_NCASE_STD */
    int mbr;            /* TL_MBR_STD */
    int enumc;          /* TL_ENUMC_STD */
    int strct;          /* TL_STRCT_STD */

    /* for preprocessor */
    int cond;           /* TL_COND_STD */
    int ppname;         /* TL_PPNAME_STD */
    int paramp;         /* TL_PARAMP_STD */
    int argp;           /* TL_ARGP_STD */
    const char *ver;    /* TL_VER_STD */
};


extern struct main_opt main_opt;            /* options */
extern struct main_tl main_tl;              /* translation limits */
extern unsigned char main_ch[UCHAR_MAX];    /* character categories */

#ifdef HAVE_ICONV
/* conversion handlers; note these are pointers */
extern iconv_t *main_iton;    /* from input to internal */
extern iconv_t *main_ntoi;    /* from internal to input */
extern iconv_t *main_ntoe;    /* from internal to exec */
extern iconv_t *main_ntow;    /* from internal to wide */
#endif    /* HAVE_ICONV */


/* macro functions */
#define main_opt()  ((const struct main_opt *)&main_opt)
#define main_tl()   ((const struct main_tl *)&main_tl)


#endif    /* MAIN_H */

/* end of main.h */
