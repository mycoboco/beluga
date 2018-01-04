/*
 *  main
 */

#include <ctype.h>         /* isalnum, isspace */
#include <errno.h>         /* errno */
#include <limits.h>        /* INT_MAX, UCHAR_MAX */
#include <stdarg.h>        /* va_list, va_start, va_end */
#include <stddef.h>        /* NULL */
#include <stdio.h>         /* FILE, fopen, stdin, stdout, stderr, fprintf, vfprintf, printf, puts,
                              fclose */
#include <stdlib.h>        /* exit, getenv, EXIT_FAILURE */
#include <string.h>        /* strcmp, strerror */
#include <cbl/assert.h>    /* assert */
#include <cbl/except.h>    /* EXCEPT_TRY, EXCEPT_EXCEPT, EXCEPT_ELSE, EXCEPT_RERAISE, EXCEPT_END */
#include <cdsl/hash.h>     /* hash_reset */
#include <cel/opt.h>       /* opt_t, opt_val_t, opt_init, opt_parse, opt_val, opt_errmsg, opt_free,
                              OPT_ARG_*, OPT_TYPE_*, OPT_CMP_* */
#ifdef HAVE_ICONV
#include <ctype.h>     /* toupper */
#include <iconv.h>     /* iconv_t, iconv_open, iconv_close */
#endif    /* HAVE_ICONV */
#ifdef HAVE_COLOR
#include <unistd.h>    /* fileno, isatty */
#endif    /* HAVE_COLOR */

#include "clx.h"
#include "common.h"
#include "cpp.h"
#include "decl.h"
#include "err.h"
#include "in.h"
#include "inc.h"
#include "ir.h"
#include "lst.h"
#include "mcr.h"
#include "strg.h"
#include "ty.h"
#include "util.h"
#include "main.h"
#include "../version.h"

#define PRGNAME  "beluga"
#define AUTHOR   "Jun Woong"
#define CONTACT  "woong.jun@gmail.com"
#define HOMEPAGE "http://code.woong.org/beluga"

#define DEFTARGET "null"    /* default target */

#ifdef HAVE_ICONV
#define DEFENC      "UTF-8"                       /* default encoding for icset, ecset and wcset */
#define EQENC(a, b) (findrep(a) == findrep(b))    /* checks if two encoding names are equivalent */
#endif    /* HAVE_ICONV */


struct main_opt main_opt = {    /* default values */
    /* common */
    NULL,    /* prgname */
    0,       /* std */
    0,       /* diagstyle */
    0,       /* path */
    0,       /* wchart */
    0,       /* logicshift */
    0,       /* uchar */
    0,       /* extension */
    0,       /* nowarn */
#ifdef HAVE_COLOR
    2,       /* color */
#endif    /* HAVE_COLOR */
    1,       /* warncode */
    0,       /* verbose */
#ifdef HAVE_ICONV
    NULL,    /* icset */
    NULL,    /* ecset */
    NULL,    /* wcset */
#endif    /* HAVE_ICONV */

    /* for compiler proper */
    1,       /* sizet */
    1,       /* ptrdifft */
    1,       /* ptrlong */
    0,       /* pfldunsign */
    0,       /* xref */
    0,       /* glevel */
    0,       /* proto */
    0,       /* unwind */
#ifndef NDEBUG
    0,       /* _debug */
#endif    /* !NDEBUG */

    /* for preprocessor */
    0,       /* pponly */
    2,       /* trigraph */
    2,       /* little_endian */
    0,       /* stricterr */
    0,       /* nostdinc */
    0,       /* onlystdmcr */
    0,       /* output */
    0,       /* pptool */
};

struct main_tl main_tl;              /* translation limits */
unsigned char main_ch[UCHAR_MAX];    /* character categories */

#ifdef HAVE_ICONV
/* conversion descriptors; defined as pointers to compare to NULL */
iconv_t *main_iton;    /* from input to internal */
iconv_t *main_ntoi;    /* from internal to input */
iconv_t *main_ntoe;    /* from internal to exec */
iconv_t *main_ntow;    /* from internal to wide */
#endif    /* HAVE_ICONV */


static int endian = 1;          /* for LITTLE from common.h */
static FILE *infile;            /* input file */
static FILE *outfile;           /* output file */
static const char *infname;     /* name of input file */
static const char *outfname;    /* name of output file */
static int optfree;             /* true while necessary to call opt_free() */

#ifdef HAVE_ICONV
/* conversion descriptors */
static iconv_t iton;
static iconv_t ntoi;
static iconv_t ntoe;
static iconv_t ntow;
#endif    /* HAVE_ICONV */


/*
 *  sets translation limits;
 *  TODO: add C11 parameters
 */
static void settl(void)
{
    switch(main_opt.std) {
        case 0:    /* non-std mode */
            /* common */
            main_tl.iname = INT_MAX;     /* not used */
            main_tl.parene = INT_MAX;    /* not used */
            main_tl.line = SZ_MAX;       /* not used */
            main_tl.lineno = SZ_MAX;     /* not used */

            /* for compiler proper */
            main_tl.block = INT_MAX;     /* not used */
            main_tl.decl = INT_MAX;      /* not used */
            main_tl.parend = INT_MAX;    /* not used */
            main_tl.ename = INT_MAX;     /* not used */
            main_tl.name = INT_MAX;      /* not used */
            main_tl.nameb = INT_MAX;     /* not used */
            main_tl.param = INT_MAX;     /* not used */
            main_tl.arg = INT_MAX;       /* not used */
            main_tl.str = SZ_MAX;        /* not used */
            main_tl.obj = SZ_MAX;        /* not used */
            main_tl.ncase = INT_MAX;     /* not used */
            main_tl.mbr = INT_MAX;       /* not used */
            main_tl.enumc = INT_MAX;     /* not used */
            main_tl.strct = INT_MAX;     /* not used */

            /* for preprocessing */
            main_tl.inc = INT_MAX;       /* not used */
            main_tl.cond = INT_MAX;      /* not used */
            main_tl.ppname = INT_MAX;    /* not used */
            main_tl.paramp = INT_MAX;    /* not used */
            main_tl.argp = INT_MAX;      /* not used */
            main_tl.ver = "0";           /* not used */
            break;
        case 1:    /* C90 */
            /* common */
            main_tl.iname = TL_INAME_C90;
            main_tl.parene = TL_PARENE_C90;
            main_tl.line = TL_LINE_C90;
            main_tl.lineno = TL_LINENO_C90;

            /* for compiler proper */
            main_tl.block = TL_BLOCK_C90;
            main_tl.decl = TL_DECL_C90;
            main_tl.parend = TL_PAREND_C90;
            main_tl.ename = TL_ENAME_C90;
            main_tl.name = TL_NAME_C90;
            main_tl.nameb = TL_NAMEB_C90;
            main_tl.param = TL_PARAM_C90;
            main_tl.arg = TL_ARG_C90;
            main_tl.str = TL_STR_C90;
            main_tl.obj = TL_OBJ_C90;
            main_tl.ncase = TL_NCASE_C90;
            main_tl.mbr = TL_MBR_C90;
            main_tl.enumc = TL_ENUMC_C90;
            main_tl.strct = TL_STRCT_C90;

            /* for preprocessor */
            main_tl.inc = TL_INC_C90;
            main_tl.cond = TL_COND_C90;
            main_tl.ppname = TL_PPNAME_C90;
            main_tl.paramp = TL_PARAMP_C90;
            main_tl.argp = TL_ARGP_C90;
            main_tl.ver = TL_VER_C90;
            break;
        case 2:    /* C99 */
            /* common */
            main_tl.iname = TL_INAME_C99;
            main_tl.parene = TL_PARENE_C99;
            main_tl.line = TL_LINE_C99;
            main_tl.lineno = TL_LINENO_C99;

            /* for compiler proper */
            main_tl.block = TL_BLOCK_C99;
            main_tl.decl = TL_DECL_C99;
            main_tl.parend = TL_PAREND_C99;
            main_tl.ename = TL_ENAME_C99;
            main_tl.name = TL_NAME_C99;
            main_tl.nameb = TL_NAMEB_C99;
            main_tl.param = TL_PARAM_C99;
            main_tl.arg = TL_ARG_C99;
            main_tl.str = TL_STR_C99;
            main_tl.obj = TL_OBJ_C99;
            main_tl.ncase = TL_NCASE_C99;
            main_tl.mbr = TL_MBR_C99;
            main_tl.enumc = TL_ENUMC_C99;
            main_tl.strct = TL_STRCT_C99;

            /* for preprocessor */
            main_tl.inc = TL_INC_C99;
            main_tl.cond = TL_COND_C99;
            main_tl.ppname = TL_PPNAME_C99;
            main_tl.paramp = TL_PARAMP_C99;
            main_tl.argp = TL_ARGP_C99;
            main_tl.ver = TL_VER_C99;
            break;
        case 3:    /* C11 */
            /* common */
            main_tl.iname = TL_INAME_C99;
            main_tl.parene = TL_PARENE_C99;
            main_tl.line = TL_LINE_C99;
            main_tl.lineno = TL_LINENO_C99;

            /* for compiler proper */
            main_tl.block = TL_BLOCK_C99;
            main_tl.decl = TL_DECL_C99;
            main_tl.parend = TL_PAREND_C99;
            main_tl.ename = TL_ENAME_C99;
            main_tl.name = TL_NAME_C99;
            main_tl.nameb = TL_NAMEB_C99;
            main_tl.param = TL_PARAM_C99;
            main_tl.arg = TL_ARG_C99;
            main_tl.str = TL_STR_C99;
            main_tl.obj = TL_OBJ_C99;
            main_tl.ncase = TL_NCASE_C99;
            main_tl.mbr = TL_MBR_C99;
            main_tl.enumc = TL_ENUMC_C99;
            main_tl.strct = TL_STRCT_C99;

            /* for preprocessor */
            main_tl.inc = TL_INC_C99;
            main_tl.cond = TL_COND_C99;
            main_tl.ppname = TL_PPNAME_C99;
            main_tl.paramp = TL_PARAMP_C99;
            main_tl.argp = TL_ARGP_C99;
            main_tl.ver = TL_VER_C99;
            break;
        default:
            assert(!"invalid standard mode -- should never reach here");
            break;
    }
}


/*
 *  initializes the table for character categories;
 *  see ISCH_* macros in common.h:
 *  - ISCH_I():  isalnum  || _
 *  - ISCH_IP(): isalnum  || _ || .
 *  - ISCH_SP(): isspace but \n
 */
static void setchcat(void)
{
    int c;

    for (c = 0; c < NELEM(main_ch); c++) {
        unsigned f = 0;
        if (isalnum(c))
            f |= (0x01 |    /* ISCH_I */
                  0x02);    /* ISCH_IP */
        if (isspace(c) && c != '\n')
            f |= 0x04;      /* ISCH_SP */
        main_ch[c] = f;
    }

    main_ch['_'] |= (0x01 |    /* ISCH_I */
                     0x02);    /* ISCH_IP */
    main_ch['.'] |= 0x02;      /* ISCH_IP */
}


#ifdef HAVE_ICONV
/*
 *  closes conversion descriptors
 */
static void closecv(void)
{
    static iconv_t **cda[] = { &main_iton, &main_ntoi, &main_ntoe, &main_ntow };

    int i;

    for (i = 0; i < NELEM(cda); i++)
        if (*cda[i])
            iconv_close(**cda[i]);
}
#endif    /* HAVE_ICONV */


/*
 *  terminates the program with clean-ups
 */
static void quit(int code)
{
#ifdef HAVE_ICONV
    /* clean-up for prepcv() */
    closecv();    /* benign */
#endif    /* HAVE_ICONV */

    /* clean-up for parseopt() */
    if (infile)
        fclose(infile);
    if (outfile)
        fclose(outfile);
    if (optfree)
        opt_free();

    exit(code);
}


/*
 *  prints messages from I/O and terminates
 */
static void ioerr(const char *file)
{
    const char *sep;
    const char *msg = strerror(errno);

    if (!file)
        file = sep = "";
    else
        sep = ": ";

    fprintf(stderr, "%s: %s%s%s\n", main_opt.prgname, file, sep, msg);

    quit(EXIT_FAILURE);
}


#ifdef HAVE_ICONV
/*
 *  prints messages from preparing encoding conversions
 */
static void cverr(const char *to, const char *from)
{
    assert(to);
    assert(from);

    fprintf(stderr, "%s: conversion from `%s' to `%s' is not supported\n", main_opt.prgname, from,
            to);
    fprintf(stderr, "%s: check what encodings your system's \"iconv\" library supports\n",
            main_opt.prgname);

    quit(EXIT_FAILURE);
}
#endif    /* HAVE_ICONV */


/*
 *  prints messages from parsing options and terminates
 */
static void oerr(const char *fmt, ...)
{
    va_list ap;

    assert(fmt);

    va_start(ap, fmt);
    fprintf(stderr, "%s: ", main_opt.prgname);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "Try `%s --help' for more information.\n", main_opt.prgname);
    va_end(ap);

    quit(EXIT_FAILURE);
}


/*
 *  displays the version information and terminates
 */
static void version(void)
{
    const char *verstr =
        "beluga: a standrd C compiler " VERSION "\n"
        "This is free software; see the LICENSE file for more information. There is NO\n"
        "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n"
        "\n"
        "Written by " AUTHOR " based on lcc developed by Chris Fraser and David Hanson.";

    puts(verstr);

    quit(0);
}


/*
 *  displays the help message and terminates
 */
static void help(void)
{
    static char *msg[] = {
      /* 12345678911234567892123456789312345678941234567895123456789612345678971234567898 */
      "\nMandatory arguments to long options are mandatory for short options too.",
        /* for preprocessor */
        "  -3, --trigraph         supports trigraphs",

        /* common */
        "      --colorize=<never|always|auto>",
        "                         use color in diagnostics",

        /* for preprocessor */
        "  -D, --define <macro>[=<val>]",
        "                         define <macro> as <val>; defined as 1 if <val> is",
        "                           omitted",
        "  -E, --preprocess-only  preprocess only; do not compile",

        /* common */
        "      --error-stop=<n>   abort compilation after <n> errors",

        /* for compiler proper */
        "      --exec-charset=<set>",
        "                         convert strings and character constants to character",
        "                           set <set>",
        "  -g, --glevel[=<n>]     ignored for now",

        /* for preprocessor */
        "  -H, --show-include-tree",
        "                         print #include hierarchy",

        /* common */
        "      --help             display this help and exit",

        /* for preprocessor */
        "  -I, --include <dir>    add <dir> to the user include path",
        "      --include-after <dir>",
        "                         append <dir> to the system include path",
        "      --include-builtin <dir>",
        "                         prepend <dir> to the system include path",
        "      --include-prefix <dir>",
        "                         append <dir> to the prefix from --include-set-prefix",
        "                           and handle as given to -I",
        "      --include-prefix-after <dir>",
        "                         append <dir> to the prefix from --include-set-prefix",
        "                           and handle as given to --include-after",
        "      --include-set-prefix <prefix>",
        "                         set <prefix> as the prefix for --include-prefix",
        "      --include-system <dir>",
        "                         prepend <dir> to the system include path",

        /* common */
        "      --input-charset=<set>",
        "                         specify the default character set for source files",

        /* for preprocessor */
        "      --list-macro-defs  list #define directives for all defined macros",

        /* common */
        "      --logical-shift    perform logical shift on right shift operation",

        /* for preprocessor */
        "      --make-deps        generate Makefile dependencies ignoring system headers",
        "      --make-deps-sys    like --make-deps but include system headers",
        "      --no-std-include   do not search the system include path for headers",

        /* common */
        "      --no-warning-code",
        "                         do not display warning codes in diagnostics",
        "  -o, --output=<file>    set output file",

        /* for preprocessor */
        "      --only-std-macros  do not predefine non-standard macros",
        "  -P, --no-linemarkers   inhibit generation of linemarkers",

        /* common */
        "      --path=<canonical|long|short>",
        "                         control how include paths are displayed",
        "      --plain-char=<signed|unsigned>",
        "                         set plain char type as signed or unsigned char",

        /* for compiler proper */
        "      --plain-int-field=<signed|unsigned>",
        "                         set int bit-field as signed or unsigned int",
        "      --pointer=<int|long>",
        "                         set pointers to be compatible with int or long int",
        "      --ptrdifft=<int|long>",
        "                         set ptrdiff_t type as int or long int",
        "      --show-prototype   output prototype information",
        "      --sizet=<uint|ulong>",
        "                         set size_t type as unsigned int or unsigned long",

        /* common */
        "      --std=<standard>   assume that the input sources are for <standard>",

        /* for preprocessor */
        "      --strict-error     #error aborts preprocessing",

        /* for compiler proper */
        "      --target=<target>  generate output code for <target>",

        /* for preprocessor */
        "      --target-endian=<host|big|little>",
        "                         set byte endian-ness of the target",
        "  -U, --undef <macro>    undefine <macro>",

        /* for compiler proper */
        "      --unwind-typedef   unwind typedef names in diagnostics",

        /* common */
        "  -v, --show-code        print source code in diagnostics",
        "      --verbose          print various compiler information",
        "      --version          output version information and exit",
        "  -W, --more-warnings    turn on additional warnings",
        "      --warning-error=<n>",
        "                         treat a warning with code <n> as an error",
        "      --warning-not-error=<n>",
        "                         treat a warning with code <n> as a warning",
        "      --warning-off=<n>  turn off a warning with code <n>",
        "      --warning-on=<n>   turn on a warning with code <n>",
        "      --wchart=<long|ushort|int>",
        "                         set wchar_t type as long, unsigned short or int",
        "      --wide-exec-charset=<set>",
        "                         convert wide strings and character constants to",
        "                           character set <set>",
        "  -w, --no-warnings      turn off all warnings",
        "  -X, --extensions       allow non-standard extensions",

        /* for compiler proper */
        "  -x, --xref             ignored for now",
    };

    int i;

    printf("Usage: %s [OPTION]... [FILE]\n", main_opt.prgname);

    for (i = 0; i < NELEM(msg); i++)
        puts(msg[i]);

    puts("\nFor bug reporting instructions, please see:\n"
         "<" HOMEPAGE ">.");

    quit(0);
}


/*
 *  parses options
 */
static void parseopt(int argc, char **argv)
{
    static opt_t tab[] = {
        /* common */
        " ",                    0,            OPT_ARG_NO,             OPT_TYPE_NO,
        "std",                  UCHAR_MAX+1,  OPT_ARG_REQ,            OPT_TYPE_STR,
        "show-code",            'v',          &(main_opt.diagstyle),  1,
        "path",                 UCHAR_MAX+2,  OPT_ARG_REQ,            OPT_TYPE_STR,
        "wchart",               UCHAR_MAX+3,  OPT_ARG_REQ,            OPT_TYPE_STR,
        "logical-shift",        0,            &(main_opt.logicshift), 1,
        "plain-char",           UCHAR_MAX+4,  OPT_ARG_REQ,            OPT_TYPE_STR,
        "extensions",           'X',          OPT_ARG_NO,             OPT_TYPE_NO,
        "more-warnings",        'W',          OPT_ARG_NO,             OPT_TYPE_NO,
        "no-warnings",          'w',          &(main_opt.nowarn),     1,
        "colorize",             UCHAR_MAX+5,  OPT_ARG_REQ,            OPT_TYPE_STR,
        "warning-on",           UCHAR_MAX+6,  OPT_ARG_REQ,            OPT_TYPE_UINT,
        "warning-off",          UCHAR_MAX+7,  OPT_ARG_REQ,            OPT_TYPE_UINT,
        "warning-error",        UCHAR_MAX+8,  OPT_ARG_REQ,            OPT_TYPE_INT,
        "warning-not-error",    UCHAR_MAX+9,  OPT_ARG_REQ,            OPT_TYPE_INT,
        "no-warning-code",      0,            &(main_opt.warncode),   0,
        "verbose",              0,            &(main_opt.verbose),    1,
        "error-stop",           UCHAR_MAX+10, OPT_ARG_REQ,            OPT_TYPE_INT,
        "output",               'o',          OPT_ARG_REQ,            OPT_TYPE_STR,
        "input-charset",        UCHAR_MAX+11, OPT_ARG_REQ,            OPT_TYPE_STR,
        "wide-exec-charset",    UCHAR_MAX+12, OPT_ARG_REQ,            OPT_TYPE_STR,
        "help",                 UCHAR_MAX+13, OPT_ARG_NO,             OPT_TYPE_NO,
        "version",              UCHAR_MAX+14, OPT_ARG_NO,             OPT_TYPE_NO,

        /* for compiler proper */
        "sizet",                UCHAR_MAX+15, OPT_ARG_REQ,            OPT_TYPE_STR,
        "ptrdifft",             UCHAR_MAX+16, OPT_ARG_REQ,            OPT_TYPE_STR,
        "pointer",              UCHAR_MAX+17, OPT_ARG_REQ,            OPT_TYPE_STR,
        "plain-int-field",      UCHAR_MAX+18, OPT_ARG_REQ,            OPT_TYPE_STR,
        "xref",                 'x',          &(main_opt.xref),       1,
        "glevel",               'g',          OPT_ARG_OPT,            OPT_TYPE_INT,
        "show-prototype",       0,            &(main_opt.proto),      1,
        "unwind-typedef",       0,            &(main_opt.unwind),     1,
        "exec-charset",         UCHAR_MAX+19, OPT_ARG_REQ,            OPT_TYPE_STR,
        "target",               UCHAR_MAX+20, OPT_ARG_REQ,            OPT_TYPE_STR,
#ifndef NDEBUG
        "_debug",               0,            &(main_opt._debug),     1,
#endif    /* !NDEBUG */

        /* for preprocessor */
        "preprocess-only",      'E',          &(main_opt.pponly),     1,
        "define",               'D',          OPT_ARG_REQ,            OPT_TYPE_STR,
        "undef",                'U',          OPT_ARG_REQ,            OPT_TYPE_STR,
        "trigraph",             '3',          &(main_opt.trigraph),   1,
        "include",              'I',          OPT_ARG_REQ,            OPT_TYPE_STR,
        "target-endian",        UCHAR_MAX+21, OPT_ARG_REQ,            OPT_TYPE_STR,
        "strict-error",         0,            &(main_opt.stricterr),  1,
        "no-std-include",       0,            &(main_opt.nostdinc),   1,
        "only-std-macros",      0,            &(main_opt.onlystdmcr), 1,
        "include-system",       UCHAR_MAX+22, OPT_ARG_REQ,            OPT_TYPE_STR,
        "include-builtin",      UCHAR_MAX+23, OPT_ARG_REQ,            OPT_TYPE_STR,
        "include-after",        UCHAR_MAX+24, OPT_ARG_REQ,            OPT_TYPE_STR,
        "include-set-prefix",   UCHAR_MAX+25, OPT_ARG_REQ,            OPT_TYPE_STR,
        "include-prefix",       UCHAR_MAX+26, OPT_ARG_REQ,            OPT_TYPE_STR,
        "include-prefix-after", UCHAR_MAX+27, OPT_ARG_REQ,            OPT_TYPE_STR,
        "no-linemarkers",       'P',          OPT_ARG_NO,             OPT_TYPE_NO,
        "show-include-tree",    'H',          OPT_ARG_NO,             OPT_TYPE_NO,
        "list-macro-defs",      UCHAR_MAX+28, OPT_ARG_NO,             OPT_TYPE_NO,
        "make-deps",            UCHAR_MAX+29, OPT_ARG_NO,             OPT_TYPE_NO,
        "make-deps-sys",        UCHAR_MAX+30, OPT_ARG_NO,             OPT_TYPE_NO,
        NULL,
    };

    int c;
    const void *argptr;
    const char *iprfx = "";

    main_opt.prgname = opt_init(tab, &argc, &argv, &argptr, PRGNAME, '/');
    if (!main_opt.prgname) {
        fprintf(stderr, "%s: failed to parse options\n", PRGNAME);
        quit(EXIT_FAILURE);
    }
    optfree = 1;
    while ((c = opt_parse()) != -1) {
        switch(c) {
            /* common */
            case UCHAR_MAX+1:    /* --std */
                {
                    opt_val_t t[] = {
                        "c89",  1, "c90", 1, "c95", 1,
                        "c99",  2,
                        "c11",  3,
                        NULL,  -1
                    };
                    main_opt.std = opt_val(t, argptr, OPT_CMP_CASEIN);
                    if (main_opt.std == -1)
                        oerr("`c89', `c90', `c95', `c99' or `c11' must be given for --std\n");
                    main_opt.extension = 0;
                    main_opt.trigraph = 3;
                    err_level = 3;
                }
                break;
            case UCHAR_MAX+2:    /* --path */
                {
                    opt_val_t t[] = {
                        "canonical",     0,
                        "non-canonical", 1, "long", 1,
                        "short",         2,
                        NULL,           -1
                    };
                    main_opt.path = opt_val(t, argptr, OPT_CMP_CASEIN | OPT_CMP_NORMSPC);
#ifndef HAVE_REALPATH
                    if (main_opt.path == 0)
                        oerr("built without HAVE_REALPATH; --path=canonical not supported\n");
#endif    /* !HAVE_REALPATH */
                    if (main_opt.path == -1)
                        oerr("`canonical', `long' or `short' must be given for --path\n");
                }
                break;
            case UCHAR_MAX+3:    /* --wchart */
                {
                    opt_val_t t[] = {
                        "long",    0,
                        "ushort",  1, "unsigned short", 1,
                        "int",     2,
                        NULL,     -1
                    };
                    main_opt.wchart = opt_val(t, argptr, OPT_CMP_CASEIN | OPT_CMP_NORMSPC);
                    if (main_opt.wchart == -1)
                        oerr("`long', `int' or `ushort' must be given for --wchart\n");
                }
                break;
            case UCHAR_MAX+4:    /* --plain-char */
                {
                    opt_val_t t[] = {
                        "signed",    0,
                        "unsigned",  1,
                        NULL,       -1
                    };
                    main_opt.uchar = opt_val(t, argptr, OPT_CMP_CASEIN);
                    if (main_opt.uchar == -1)
                        oerr("`signed' or `unsigned' must be given for --plain-char\n");
                }
                break;
            case 'X':    /* --extensions */
                main_opt.extension = 1;
                main_opt.std = 0;
                break;
            case 'W':    /* --more-warnings */
                if (err_level < 2)
                    err_level++;
                break;
            case UCHAR_MAX+5:    /* --colorize */
#ifdef HAVE_COLOR
                {
                    opt_val_t t[] = {
                        "off",   0, "never",  0,
                        "on",    1, "always", 1,
                        "auto",  2,
                        NULL,   -1
                    };
                    main_opt.color = opt_val(t, argptr, OPT_CMP_CASEIN);
                    if (main_opt.color == -1)
                        oerr("`on', `off' or `auto' must be given for --sizet\n");
                }
#else    /* !HAVE_COLOR */
                oerr("built without HAVE_COLOR; --colorize not supported\n");
#endif    /* HAVE_COLOR */
                break;
            case UCHAR_MAX+6:    /* --warning-on */
                err_setwarn(*(unsigned long *)argptr, 0);
                break;
            case UCHAR_MAX+7:    /* --warning-off */
                err_setwarn(*(unsigned long *)argptr, 1);
                break;
            case UCHAR_MAX+8:    /* --warning-error */
                err_setwarn(*(long *)argptr, 2);
                break;
            case UCHAR_MAX+9:    /* --warning-not-error */
                err_setwarn(*(long *)argptr, 3);
                break;
            case UCHAR_MAX+10:    /* --error-stop */
                err_lim = *(const long *)argptr;
                if (err_lim < 0)
                    oerr("error-stop must be non-negative\n");
                break;
            case 'o':             /* --output */
                if (!(((const char *)argptr)[0] == '-' && ((const char *)argptr)[1] == '\0'))
                    outfname = argptr;
                break;
           case UCHAR_MAX+11:    /* --input-charset */
#ifdef HAVE_ICONV
                main_opt.icset = (const char *)argptr;
#else    /* !HAVE_ICONV */
                oerr("built without HAVE_ICONV; --input-charset not supported\n");
#endif    /* HAVE_ICONV */
                break;
            case UCHAR_MAX+12:    /* --wide-exec-charset */
#ifdef HAVE_ICONV
                main_opt.wcset = (const char *)argptr;
#else    /* !HAVE_ICONV */
                oerr("built without HAVE_ICONV; --wide-exec-charset not supported\n");
#endif    /* HAVE_ICONV */
                break;
            case UCHAR_MAX+13:    /* --help */
                help();
                break;
            case UCHAR_MAX+14:    /* --version */
                version();
                break;

            /* for compiler proper */
            case UCHAR_MAX+15:    /* --sizet */
                {
                    opt_val_t t[] = {
                        "uint",   0, "unsigned",      0, "unsigned int",  0,
                        "ulong",  1, "unsigned long", 1,
                        NULL,    -1
                    };
                    main_opt.sizet = opt_val(t, argptr, OPT_CMP_CASEIN | OPT_CMP_NORMSPC);
                    if (main_opt.sizet == -1)
                        oerr("`uint' or `ulong' must be given for --sizet\n");
                }
                break;
            case UCHAR_MAX+16:    /* --ptrdifft */
                {
                    opt_val_t t[] = {
                        "int",   0,
                        "long",  1,
                        NULL,   -1
                    };
                    main_opt.ptrdifft = opt_val(t, argptr, OPT_CMP_CASEIN);
                    if (main_opt.ptrdifft == -1)
                        oerr("`int' or `long' must be given for --ptrdifft\n");
                }
                break;
            case UCHAR_MAX+17:    /* --pointer */
                {
                    opt_val_t t[] = {
                        "int",   0,
                        "long",  1,
                        NULL,   -1
                    };
                    main_opt.ptrlong = opt_val(t, argptr, OPT_CMP_CASEIN);
                    if (main_opt.ptrlong == -1)
                        oerr("`int' or `long' must be given for --pointer\n");
                }
                break;
            case UCHAR_MAX+18:    /* --plain-int-field */
                {
                    opt_val_t t[] = {
                        "signed",    0,
                        "unsigned",  1,
                        NULL,       -1
                    };
                    main_opt.pfldunsign = opt_val(t, argptr, OPT_CMP_CASEIN);
                    if (main_opt.pfldunsign == -1)
                        oerr("`signed' or `unsigned' must be given for --plain-int-field\n");
                }
                break;
            case 'g':    /* --glevel */
                if (argptr) {
                    main_opt.glevel = *(const long *)argptr;
                    if (main_opt.glevel < 0)
                        oerr("debugging level must be non-negative\n");
                } else
                    main_opt.glevel = 1;
                break;
            case UCHAR_MAX+19:    /* --exec-charset */
#ifdef HAVE_ICONV
                main_opt.ecset = (const char *)argptr;
#else    /* !HAVE_ICONV */
                oerr("built without HAVE_ICONV; --exec-charset not supported\n");
#endif    /* HAVE_ICONV */
                break;
            case UCHAR_MAX+20:    /* --target */
                ir_cur = ir_bind((const char *)argptr);
                if (!ir_cur)
                    oerr("`%s' is not supported target\n", (const char *)argptr);
                break;

            /* for preprocessor */
            case 'D':    /* --define */
                mcr_addcmd(argptr);
                break;
            case 'U':    /* --undef */
                mcr_delcmd(argptr);
                break;
            case 'I':    /* --include */
                inc_add("", argptr, 0);
                break;
            case UCHAR_MAX+21:    /* --target-endian */
#ifdef HAVE_ICONV
                {
                    opt_val_t t[] = {
                        "host",   2,
                        "big",    0,
                        "little", 1,
                        NULL,     -1
                    };
                    main_opt.little_endian = opt_val(t, argptr, OPT_CMP_CASEIN);
                    if (main_opt.little_endian == -1)
                        oerr("`host', `big' or `little' must be given for --target-endian\n");
                }
#else    /* !HAVE_ICONV */
                oerr("built without HAVE_ICONV; --target-endian not supported\n");
#endif    /* HAVE_ICONV */
                break;
            case UCHAR_MAX+22:    /* --include-system */
                inc_add("", argptr, 1);
                break;
            case UCHAR_MAX+23:    /* --include-builtin */
                inc_add("", argptr, 2);
                break;
            case UCHAR_MAX+24:    /* --include-after */
                inc_add("", argptr, 3);
                break;
            case UCHAR_MAX+25:    /* --include-set-prefix */
                iprfx = argptr;
                break;
            case UCHAR_MAX+26:    /* --include-prefix */
                inc_add(iprfx, argptr, 0);
                break;
            case UCHAR_MAX+27:    /* --include-prefix-after */
                inc_add(iprfx, argptr, 3);
                break;
            case 'P':    /* --no-linemarkers */
                main_opt.output = 1;
                break;
            case 'H':    /* --show-include-tree */
                main_opt.pptool = 1;
                main_opt.output = 2;
                break;
            case UCHAR_MAX+28:    /* --list-macro-defs */
                main_opt.pptool = 2;
                main_opt.output = 2;
                break;
            case UCHAR_MAX+29:    /* --make-deps */
                main_opt.pptool = 3;
                main_opt.output = 2;
                break;
            case UCHAR_MAX+30:    /* --make-deps-sys */
                main_opt.pptool = 4;
                main_opt.output = 2;
                break;

            /* common case labels follow */
            case 0:    /* flag variable set; do nothing else now */
                break;
            case '?':    /* unrecognized option */
                assert(!"impossible return value -- should never reach here");
                /* no break */
            case '-':    /* no or invalid argument given for option */
            case '+':    /* argument given to option that takes none */
            case '*':    /* ambiguous option */
                oerr(opt_errmsg(c), (const char *)argptr, opt_ambmstr());
                break;
            default:
                assert(!"not all options covered -- should never reach here");
                break;
        }
    }

#ifdef HAVE_COLOR
    if (main_opt.color == 2) {
        const char *p = getenv("TERM");
        main_opt.color = isatty(fileno(stderr)) && (!p || strcmp(p, "dumb") != 0);
    }
#endif    /* HAVE_COLOR */

    if (!ir_cur)
        ir_cur = ir_bind(DEFTARGET);    /* sets to default binding */
    ir_cur->option(&argc, &argv, oerr);

    if (main_opt.little_endian == 2)
        main_opt.little_endian = LITTLE;

    infile = stdin;
    if (argc > 1 && strcmp(argv[1], "-") != 0) {
        infname = argv[1];
        errno = 0;
        if ((infile = fopen(infname, "r")) == NULL)
            ioerr(infname);
    }
    outfile = stdout;
    if (outfname && (outfile = fopen(outfname, "w")) == NULL)
        ioerr(outfname);

    opt_free();
    optfree = 0;
}


#ifdef HAVE_ICONV
/*
 *  finds the representive name for an encoding
 */
static const char *findrep(const char *enc)
{
    static const char *alias[] = { "UTF-8", "UTF8", NULL,
                                   NULL };

    int i;
    const char *cur;

    assert(enc);
    assert(strcmp(alias[0], DEFENC) == 0);

    cur = alias[0];
    for (i = 0; i < NELEM(alias); i++) {
        const char *p = enc, *q = alias[i];
        for (; toupper(*p) == *q; p++, q++)
            if (*p == '\0')
                break;
        if (*p == *q)
            break;
        if (!alias[i+1]) {
            i++;
            cur = alias[i+1];
            if (!cur)
                break;
        }
    }

    return cur;
}


/*
 *  prepares conversion descriptors;
 *  necessary descriptors and their meanings are:
 *
 *                                            +--(ntoe)--> exec
 *    input <--(iton/ntoi)--> native(UTF-8) --+
 *                                            +--(ntow)--> wide
 *
 *  ASSUMPTION: (HAVE_ICONV) UTF-8 used as internal pivot encoding
 */
static void prepcv(void)
{
    const char *from;

    /* conversion descriptor from input to internal */
    if (main_opt.icset && !EQENC(DEFENC, main_opt.icset)) {
        if ((iton = iconv_open("UTF-8", main_opt.icset)) == (iconv_t)-1)
            cverr("UTF-8", main_opt.icset);
        main_iton = &iton;
        if ((ntoi = iconv_open(main_opt.icset, "UTF-8")) == (iconv_t)-1)
            cverr(main_opt.icset, "UTF-8");
        main_ntoi = &ntoi;
        from = "UTF-8";
    } else
        from = main_opt.icset = DEFENC;
    assert(main_opt.icset);

    /* conversion descriptor from internal to exec */
    if ((main_opt.ecset || (main_opt.ecset=DEFENC, iton)) && !EQENC(main_opt.ecset, from)) {
        if ((ntoe = iconv_open(main_opt.ecset, from)) == (iconv_t)-1)
            cverr(main_opt.ecset, from);
        main_ntoe = &ntoe;
    }

    /* conversion descriptor from internal to wide */
    if ((main_opt.wcset || (main_opt.wcset=DEFENC, iton)) && !EQENC(main_opt.wcset, from)) {
        if ((ntow = iconv_open(main_opt.wcset, from)) == (iconv_t)-1)
            cverr(main_opt.wcset, from);
        main_ntow = &ntow;
    }
}
#endif    /* HAVE_ICONV */


/*
 *  prints messages for internal compiler errors
 */
static void printice(void)
{
    fprintf(stderr, "%s: internal error occurred with no way to recover\n", main_opt.prgname);
    fprintf(stderr, "%s: (Please report this error to %s)\n", main_opt.prgname, CONTACT);
}


/*
 *  reads environment variables for #include paths
 */
static void readenv(void)
{
    static const char *env[] = {
        "CPATH",
        "C_INCLUDE_PATH"
    };

    int i;
    char *p;

    for (i = 0; i < NELEM(env); i++) {
        p = getenv(env[i]);
        if (p)
            inc_add("", p, i);
    }
}


/*
 *  main function
 */
int main(int argc, char *argv[])
{
    int ice = 0;    /* true if ice occurred */

    xinit();

    EXCEPT_TRY
        parseopt(argc, argv);
#ifdef HAVE_ICONV
        prepcv();
#endif    /* HAVE_ICONV */
        readenv();

        settl();
        setchcat();

        strg_init();
        err_init();
        ty_init();
        in_init(infile, infname);
        mcr_init();
        inc_init();
        if (main_opt()->pponly) {
            switch(main_opt.output) {
                case 0:    /* normal */
                    cpp_start(outfile);
                    break;
                case 1:    /* no linemarkers */
                    cpp_nosync(outfile);
                    break;
                case 2:    /* no output */
                    cpp_nout();
                    break;
                default:
                    assert(!"invalid output mode -- should never reach here");
                    break;
            }
        } else {
            clx_init();
            clx_tc = clx_next();
            ir_cur->progbeg(outfile);
            decl_program();
            decl_finalize();
            ir_cur->progend();
        }
        if (err_chkwarn(ERR_PP_UNUSEDMCR))
            mcr_unused();
        switch(main_opt()->pptool) {
            case 2:    /* --list-macro-defs */
                mcr_listdef(stdout);
                break;
            case 3:    /* --make-deps */
            case 4:    /* --make-deps-sys */
                inc_mkdep(outfile);
                break;
        }
    EXCEPT_EXCEPT(err_except)    /* too many errors */
        /* nothing to do */ ;
    EXCEPT_ELSE
        ice = 1;
        cpp_close();
        printice();
#if 1
        EXCEPT_RERAISE;
#endif
    EXCEPT_END

    EXCEPT_TRY    /* tries to clean up */
        cpp_close();
        lst_free();
        in_close();
        mcr_free();
        inc_free();
        err_close();
        strg_close();
        hash_reset();
        snbuf(-1, 0);
    EXCEPT_ELSE
        if (!ice)
            printice();
#if 1
        EXCEPT_RERAISE;
#endif
    EXCEPT_END

    quit((err_count() == 0)? 0: EXIT_FAILURE);
    return 0;    /* to stop warnings */
}

/* end of main.c */
