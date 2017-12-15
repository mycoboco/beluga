/*
 *  option definitions
 */

dd("-help",    NULL, "display this help and exit")
dd("-version", NULL, "output version information and exit")

xx("E", "-E", NULL, NULL, NULL, "preprocess only; do not compile, assemble or link")

dd("c",  NULL,         "compile and assemble; do not link")
dd("S",  NULL,         "compile only; do not assemble or link")
dd("l",  " <library>", "link <library>")
dd("o",  " <file>",    "place the output into <file>")
dd("v",  NULL,         "show the programs invoked by bcc")
dd("Wp", ",<options>", "pass comma-separated options to the preprocessor")
dd("Wc", ",<options>", "pass comma-separated options to the compiler")
dd("Wa", ",<options>", "pass comma-separated options to the assembler")
dd("Wl", ",<options>", "pass comma-separated options to the linker")

xx("fsigned-char",   "--plain-char=signed",   NULL, NULL, NULL, "treat plain char as signed")
xx("funsigned-char", "--plain-char=unsigned", NULL, NULL, NULL, "treat plain char as unsigned")

tt("The following options control preprocessing:")
xx("D?",          "-D $",               NULL, NULL, "<macro>[=<value>]", "define <macro> as <value>")
xx("U?",          "-U $",               NULL, NULL, "<macro>",           "undefine <macro>")
xx("I?",          "-I $",               NULL, NULL, "<dir>",             "add <dir> to the user include path list")
xx("isystem ?",   "--include-system $", NULL, NULL, "<dir>",             "prepend <dir> to the system include path list")
xx("idirafter ?", "--include-after $",  NULL, NULL, "<dir>",             "append <dir> to the system include path list")
xx("nostdinc",    "--nostdinc",         NULL, NULL, NULL,                "don't search the system include paths")
xx("trigraphs",   "-3",                 NULL, NULL, NULL,                "recognizes and replace trigraphs")

tt("The following options control compilation:")
xx("extension", "-X", NULL, NULL, NULL, "enable non-conforming extensions")

tt("The following options control linking:")
xx("L?", NULL, "-L $", NULL, "<dir>", "add <dir> to the library path list")

tt("The following options control diagnostics:")
xx("ansi",            "-W --std=c90"
                      " -D__STRICT_ANSI__", NULL, NULL,   NULL,         "synonym for -std=c90")
xx("std=",            NULL,                 NULL, escape, "<standard>", "assume that the input sources are for <standard>; imply -Wall")
xx("unwind-typedefs", "--unwind-typedef",   NULL, NULL,   NULL,         "unwind typedefs")
xx("W",               "-W",                 NULL, NULL,   NULL,         "turn on additional warnings; use -Wextra")
xx("Wextra",          "-W",                 NULL, NULL,   NULL,         "turn on additional warnings")
xx("Wall",            "-WW",                NULL, NULL,   NULL,         "turn on most warnings; imply -Wextra")
xx("Werror",          "--werr=-3",          NULL, NULL,   NULL,         "treat all warnings as errors")
xx("Werror=",         NULL,                 NULL, escape, "<warning>",  "treat <warning> as an error")
xx("Wno-error",       "--wnerr=-3",         NULL, NULL,   NULL,         "treat all warnings as warnings")
xx("Wno-error=",      NULL,                 NULL, escape, "<warning>",  "treat <warning> as a warning")

tt("The following options turn on specific warnings; -Wno- options turns off:")
ww("backslash-newline-escape", 1, INPUT_BSSPACENL,  "backslash-newlines separated by space")
ww("combined-headers",         1, PP_COMBINEHDR,    "combining headers")
ww("comment",                  2, LEX_CMTINCMT _
                                  LEX_BSNLINCMT,    "problematic comments")
ww("comments",                 2, LEX_CMTINCMT _
                                  LEX_BSNLINCMT,    "problematic comments")
ww("cpp",                      1, PP_WARNING,       "warnings by #warning")
ww("div-by-zero",              1, EXPR_DIVBYZERO,   "division by zero")
ww("extra-braces-scalar-init", 1, PARSE_EXTRABRACE, "extra braces for a scalar initializer")
ww("expansion-to-defined",     1, PP_DEFFROMMCR,    "`defined' from macro expansion")
ww("long-logical-source-line", 1, INPUT_LONGLINE,   "long logical lines")
ww("missing-prototype",        1, PARSE_NOPROTO,    "non-prototype declarations")
ww("negate-unsigned",          1, EXPR_NEGUNSIGNED, "negating unsigned integers")
ww("null-character",           1, INPUT_EMBEDNUL,   "embedded null characters")
ww("plain-int-bitfield",       1, PARSE_PINTFLD,    "plain-int bit-fields")
ww("shadow",                   1, PARSE_HIDEID,     "an identifier shadowing another identifier")
ww("token-paste-order",        1, PP_ORDERDS,       "depending on the order of token pasting")
ww("undef",                    1, PP_EXPRUNDEFID,   "evaluation of an undefined macro in #if-like directives")
ww("undef-undefined",          1, PP_UNDEFMCR,      "undefining already undefined macros")

#undef dd
#undef tt
#undef xx
#undef ww

/* end of opt.h */
