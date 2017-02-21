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
dd("v",  NULL,         "show programs invoked by bcc")
dd("Wp", ",<options>", "pass comma-separated options to the preprocessor")
dd("Wc", ",<options>", "pass comma-separated options to the compiler")
dd("Wa", ",<options>", "pass comma-separated options to the assembler")
dd("Wl", ",<options>", "pass comma-separated options to the linker")

xx("fsigned-char",   "--plain-char=signed",   NULL, NULL, NULL, "treat plain char as signed")
xx("funsigned-char", "--plain-char=unsigned", NULL, NULL, NULL, "treat plain char as unsigned")

tt("The following options control preprocessing:")
xx("D?",          "-D $",               NULL, NULL, "<macro>[=<value>]", "define <macro> as <value>")
xx("U?",          "-U $",               NULL, NULL, "<macro>",           "undefine <macro>")
xx("I?",          "-I $",               NULL, NULL, "<dir>",             "add <dir> to the end of the user include path")
xx("isystem ?",   "--include-system $", NULL, NULL, "<dir>",             "add <dir> to the end of the system include path")
xx("idirafter ?", "--include-after $",  NULL, NULL, "<dir>",             "add <dir> to the end of the include path")
xx("nostdinc",    "--nostdinc",         NULL, NULL, NULL,                "don't search system include path")

tt("The following options control linking:")
xx("L?", NULL, "-L $", NULL, "<dir>", "add <dir> to the end of the library path")

tt("The following options control diagnostics:")
xx("ansi",            "-W --std=c90"
                      " -D__STRICT_ANSI__", NULL, NULL,   NULL,         "synonym for -std=c90")
xx("std=",            NULL,                 NULL, escape, "<standard>", "assume that the input sources are for <standard>; imply -Wall")
xx("unwind-typedefs", "--unwind-typedef",   NULL, NULL,   NULL,         "unwind typedefs")
xx("W",               "-W",                 NULL, NULL,   NULL,         "turn on additional warnings; use -Wextra")
xx("Wextra",          "-W",                 NULL, NULL,   NULL,         "turn on additional warnings")
xx("Wall",            "-WW",                NULL, NULL,   NULL,         "turn on most warnings; imply -Wextra")
xx("Werror",          "--warnerr",          NULL, NULL,   NULL,         "treat all warnings as errors")

tt("The following options turn on specific warnings; -Wno- options turns off:")
ww("combined-headers",         PP_COMBINEHDR,     "combining headers")
ww("div-by-zero",              EXPR_DIVBYZERO,    "division by zero")
ww("extra-braces-scalar-init", PARSE_EXTRABRACE,  "extra braces for a scalar initializer")
ww("long-logical-source-line", INPUT_LONGLINE,    "long logical line")
ww("negate-unsigned",          EXPR_NEGUNSIGNED,  "negating unsigned integers")
ww("token-paste-order",        PP_ORDERDS,        "depending on the order of token paste")
ww("plain-int-bitfield",       PARSE_PINTFLD,     "plain-int bit-fields")
ww("missing-prototype",        PARSE_NOPROTO,     "non-prototype declaration")
ww("shadow",                   PARSE_HIDEID,      "an identifier shadowing another identifier")
ww("undef",                    PP_EXPRUNDEFID,    "evaluation of an undefined macro in #if-like directives")
ww("undef-undefined",          PP_UNDEFMCR,       "undefining already undefined macros")

#undef dd
#undef tt
#undef xx
#undef ww

/* end of opt.h */
