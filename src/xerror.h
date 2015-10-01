/*
 *  diagnostic messages
 */

/* E: error (warning if not set)
   N: note (warning if not set)
   P: prints locus
   O: issued once for file (works only with warnings)
   U: issued once for func (works only with warnings; must be defined as yy())
   T: issued once for tree (works only with warnings; must be defined as yy())
   F: fatal errors (not set with warnings)
   W: issued if -W given or in standard mode
   A: issued if in C90 mode
   B: issued if in C99 mode
   C: issued if in C1X mode

   ASSUMPTION: (HAVE_ICONV) UTF-8 used as internal pivot encoding */

xx(INPUT_ERROR,       E    |F    , "failed to read input"                                         )
xx(INPUT_EMPTYFILE,         A|B|C, "ISO C forbids an empty source file"                           )
xx(INPUT_NOTENDNL,          W    , "input does not end in newline"                                )
xx(INPUT_BSNLEOF,           W    , "backslash-newline followed by end-of-file"                    )
xx(INPUT_LINESPLICE,  E|P        , "line splicing is not supported"                               )
xx(INPUT_LONGLINE,      P  |W    , "logical source line is too long"                              )
xx(INPUT_LONGLINESTD, N    |A|B|C, "ISO C guarantees at most %u characters"                       )
xx(INPUT_CONVFAIL,    E|P  |F    , "conversion failed while reading input"                        )

xx(PP_UNCLOSESTR,     E|P        , "missing closing %c"                                           )
xx(PP_UNCLOSEHDR,     E|P        , "missing closing %c"                                           )
xx(PP_UNCLOSECMT,     E|P        , "unclosed comment"                                             )
xx(PP_C99CMT,           P|O|A    , "C90 does not support //-comments"                             )
xx(PP_CMTINCMT,         P  |W    , "`/*' within comment"                                          )
xx(PP_UNKNOWNDIR,     E|P        , "unrecognized directive"                                       )
xx(PP_UNKNOWNDIRW,      P        , "unrecognized directive"                                       )
xx(PP_SPHTDIREC,        P  |W    , "only space and horizontal tab allowed in directives"          )
xx(PP_EXTRATOKEN,       P        , "extraneous tokens ignored"                                    )
xx(PP_EXTRATOKENCL,     P        , "extraneous tokens after `%s' ignored"                         )
xx(PP_INCBROKEN,                0, "tracking nested #include files failed"                        )
xx(PP_INCNOTENTER,              0, "file `%s' left but not entered"                               )
xx(PP_INCNOTLEAVE,              0, "file `%s' entered but not left"                               )
xx(PP_NOUNIQUELINE,             0, "different lines have the same locus"                          )
xx(PP_NOHEADER,       E|P        , "`#include' expects a header name (\"header\" or <header>)"    )
xx(PP_NOINCFILE,      E|P  |F    , "cannot open #include file `%s'"                               )
xx(PP_MANYINC1,       E|P  |F    , "too many nesting levels of `#include'"                        )
xx(PP_MANYINC2,         P  |W    , "too many nesting levels of `#include'"                        )
xx(PP_MANYINCSTD,     N    |A|B|C, "ISO C guarantees only %d nesting levels"                      )
xx(PP_COMBINEHDR,       P  |A|B|C, "combining header name from tokens is not portable"            )
xx(PP_NOMCRID,        E|P        , "missing identifier for macro name"                            )
xx(PP_MCRREDEF,       E|P        , "redefinition of macro `%s' previously defined at %p"          )
xx(PP_PMCRREDEF,      E|P        , "redefinition of built-in macro `%s'"                          )
xx(PP_PMCRUNDEF,      E|P        , "undefining built-in macro `%s'"                               )
xx(PP_UNDEFMCR,         P  |W    , "#undefining undefined macro `%s'"                             )
xx(PP_NOPNAME,        E|P        , "missing identifier for macro parameter name"                  )
xx(PP_NOPRPAREN,      E|P        , "missing `)' in macro parameter list"                          )
xx(PP_NOSPACE,          P        , "missing space between macro `%s' and replacement list"        )
xx(PP_NOEQCL,           P        , "missing `=' between %s-like macro `%s' and replacement list"  )
xx(PP_DUPNAME,        E|P        , "duplicate macro parameter `%s'"                               )
xx(PP_MANYPARAM,        P  |W    , "too many parameters"                                          )
xx(PP_MANYPSTD,       N    |A|B|C, "ISO C guarantees only %d parameters"                          )
xx(PP_MANYPPID,         P  |W    , "too many macros simultaneously defined"                       )
xx(PP_MANYPPIDSTD,    N    |A|B|C, "ISO C guarantees only %d macros"                              )
xx(PP_LONGID,           P  |W    , "identifier is too long; see `%s' declared at %p"              )
xx(PP_LONGIDSTD,      N    |A|B|C, "ISO C guarantees only %d significant characters"              )
xx(PP_DSHARPPOS,      E|P        , "`##' cannot appear at the boundaries of macro expansion"      )
xx(PP_TWODSHARP,      E|P        , "`##' cannot be an operand of `##'"                            )
xx(PP_NEEDPARAM,      E|P        , "`#' must be followed by a macro parameter"                    )
xx(PP_EMPTYARG,         P  |A    , "C90 does not support empty macro argument to macro `%s'"      )
xx(PP_DIRECINARG,       P        , "directive-like line given as macro argument"                  )
xx(PP_UNTERMARG,      E|P        , "unterminated argument list to macro `%s'"                     )
xx(PP_MANYARG1,       E|P        , "too many arguments to macro `%s'"                             )
xx(PP_MANYARG2,         P  |W    , "too many arguments to macro `%s'"                             )
xx(PP_MANYARGSTD,     N    |A|B|C, "ISO C guarantees only %d arguments"                           )
xx(PP_INSUFFARG,      E|P        , "insufficient number of arguments to macro `%s'"               )
xx(PP_INVTOKMADE,     E|P        , "`##' generated multiple tokens from `%s'"                     )
xx(PP_EMPTYTOKMADE,     P  |A    , "`##' generated an empty token"                                )
xx(PP_INVSTRMADE,     E|P        , "`#' generated an invalid string `%s'"                         )
xx(PP_ORDERSDS,         P  |W    , "evaluation order of `#' and `##' operators is unspecified"    )
xx(PP_ORDERDS,          P  |W    , "evaluation order of `##' operators is unspecified"            )
xx(PP_ORDERDSEX,      N    |W    , "no valid token can be generated from `%s'"                    )
xx(PP_UNSPCRECUR,       P  |W    , "unspecified whether call to `%s' is considered recursive"     )
xx(PP_MANYCOND,         P  |W    , "too many nesting levels of conditional inclusion"             )
xx(PP_MANYCONDSTD,    N    |A|B|C, "ISO C guarantees only %d nesting levels"                      )
xx(PP_UNTERMCOND,     E|P        , "unterminated `%C' started at %p"                              )
xx(PP_NOMATCHIF,      E|P        , "missing matching `#if' for `%s'"                              )
xx(PP_ELIFAFTRELSE,   E|P        , "`#elif' after `#else' at %p"                                  )
xx(PP_DUPELSE,        E|P        , "`#else' already seen at %p"                                   )
xx(PP_NOIFEXPR,       E|P        , "missing controlling expression for `%s'"                      )
xx(PP_NOIFID,         E|P        , "missing identifier for `%C'"                                  )
xx(PP_ILLOP,          E|P        , "`%s' is not allowed in preprocessing expression"              )
xx(PP_ILLOPW,           P  |W    , "`%s' is not allowed in preprocessing expression"              )
xx(PP_DEFFROMMCR,       P        , "`defined' generated from macro expansion"                     )
xx(PP_MCRDEF,         E|P        , "`defined' cannot be #defined"                                 )
xx(PP_NODEFID,        E|P        , "missing identifier for `defined'"                             )
xx(PP_NODEFRPAREN,    E|P        , "missing `)' for `defined'"                                    )
xx(PP_ILLEXPR,        E|P        , "invalid preprocessing expression"                             )
xx(PP_NOEXPRLPAREN,   E|P        , "missing `(' in preprocessing expression"                      )
xx(PP_EXPRERR,        E|P        , "%s expected before `%s'"                                      )
xx(PP_PPNUMBER,       E|P        , "invalid pp-number `%s'"                                       )
xx(PP_OVFCONST,         P        , "overflow in preprocessing expression"                         )
xx(PP_DIVBYZERO,        P        , "divide by zero"                                               )
xx(PP_NEGTOUNSIGN,      P  |W    , "negative value converted to unsigned type"                    )
xx(PP_OVERSHIFTS,       P        , "shift by %d is undefined"                                     )
xx(PP_OVERSHIFTU,       P        , "shift by %u is undefined"                                     )
xx(PP_RSHIFTNEG,        P  |W    , "right shift of negative value is not portable"                )
xx(PP_LSHIFTNEG,        P  |W    , "left shift of negative value is undefined"                    )
xx(PP_NEGUNSIGNED,      P  |W    , "unsigned operand of unary -"                                  )
xx(PP_LARGEHEX,       E|P        , "the value of hexadecimal escape sequence is too large"        )
xx(PP_ESCOCT89,         P  |W    , "8 and 9 are not allowed in octal escape sequence"             )
xx(PP_ESCOCT3DIG,       P  |W    , "octal escape sequence takes at most 3 digits"                 )
xx(PP_LARGEOCT,       E|P        , "the value of octal escape sequence is too large"              )
xx(PP_INVESC1,        E|P        , "invalid escape sequence `%s' in %s"                           )
xx(PP_INVESC2,        E|P        , "invalid escape sequence in %s"                                )
xx(PP_EMPTYCHAR,      E|P        , "empty character constant is not allowed"                      )
xx(PP_LARGECHAR,      E|P        , "excess characters in character constant"                      )
xx(PP_WIDENOTFIT,       P        , "wide string/character does not fit to wchar_t"                )
xx(PP_CONVFAIL,       E|P  |F    , "conversion failed while recognizing string/character"         )
xx(PP_MANYPE,           P  |W    , "too many nesting levels of parenthesized expressions"         )
xx(PP_MANYPESTD,      N    |A|B|C, "ISO C guarantees at most %d levels"                           )
xx(PP_EXPRUNDEFID,      P  |W    , "undefined identifier `%s' evaluated to 0"                     )
xx(PP_ERROR1,         E|P        , "#error%s"                                                     )
xx(PP_ERROR2,         E|P  |F    , "#error%s"                                                     )
xx(PP_UNKNOWNPRAGMA,    P        , "unknown #pragma"                                              )
xx(PP_LARGELINE,        P        , "line number is too large"                                     )
xx(PP_ZEROLINE,         P        , "line number zero is not permitted"                            )
xx(PP_ESCINFNAME,       P        , "escape sequence used in file name"                            )
xx(PP_NOLINENO,      E|P         , "missing line number for #line"                                )
xx(PP_ILLLINENO,     E|P         , "invalid line number `%s'"                                     )
xx(PP_ILLFNAME,      E|P         , "invalid file name `%s'"                                       )

#ifndef SEA_CANARY
xx(CONST_LONGSTR,       P  |W    , "string literal is too long"                                   )
xx(CONST_LONGSTRSTD,  N    |A|B|C, "ISO C guarantees only %u characters"                          )
xx(CONST_LARGECHAR,   E|P        , "excess characters in character constant"                      )
xx(CONST_EMPTYCHAR,   E|P        , "empty character constant is not allowed"                      )
xx(CONST_MBWIDE,        P  |A    , "adjacent multibyte and wide strings result in wide string"    )
xx(CONST_MBWIDESTD,   N    |A    , "the concatenation is not portable in C90"                     )
xx(CONST_WIDENOTFIT,    P        , "wide string/character does not fit to wchar_t"                )
xx(CONST_LARGEOCT,    E|P        , "the value of octal escape sequence is too large"              )
xx(CONST_LARGEHEX,    E|P        , "the value of hexadecimal escape sequence is too large"        )
xx(CONST_ILLHEXESC,   E|P        , "invalid hexadecimal escape sequence"                          )
xx(CONST_UNKNOWNESC,  E|P        , "unknown escape sequence"                                      )
xx(CONST_PPNUMBER,    E|P        , "pp-number that is not a valid %t"                             )
xx(CONST_ILLFPCNST,   E|P        , "invalid floating constant"                                    )
xx(CONST_LARGEFP,     E|P        , "the value of floating constant is too large"                  )
xx(CONST_LONGFP,      E|P        , "floating constant is too long"                                )
xx(CONST_TRUNCFP,       P        , "floating constant truncated to 0"                             )
xx(CONST_ILLINTCNST,  E|P        , "invalid integer constant"                                     )
xx(CONST_LARGEINT,    E|P        , "the value of integer constant is too large"                   )
xx(CONST_ILLOCTESC,   E|P        , "8 and 9 are not allowed in octal constant"                    )
xx(CONST_CONVFAIL,    E|P  |F    , "conversion failed while recognizing string/character"         )
xx(CONST_ESCOCT89,      P  |W    , "8 and 9 are not allowed in octal escape sequence"             )
xx(CONST_ESCOCT3DIG,    P  |W    , "octal escape sequence takes at most 3 digits"                 )

xx(LEX_INVCHAR,       E|P        , "invalid character `%s' ignored"                               )
xx(LEX_INVCHARCV,     E|P        , "invalid character `%s' ignored (after conversion to UTF-8)"   )
xx(LEX_STRAYWS,         P        , "stray whitespace character `%s'"                              )
xx(LEX_STRAYBS,       E|P        , "stray backslash character"                                    )
xx(LEX_SHARP,         E|P        , "# or ## is supported only in preprocessing"                   )
xx(LEX_EXTRATOKEN,      P  |O    , "extraneous tokens ignored"                                    )
xx(LEX_LONGIDOV,      E|P        , "identifier truncated; %P ignored"                             )
xx(LEX_LONGID,          P  |W    , "identifier is too long; see `%s' declared at %p"              )
xx(LEX_LONGEID,         P  |W    , "external identifier is too long; see `%s' declared at %p"     )
xx(LEX_LONGIDSTD,     N    |A|B|C, "ISO C guarantees only %d significant characters"              )
xx(LEX_EXTRACOMMA,    E|P        , "extra comma or missing %s"                                    )

xx(PARSE_ERROR,       E|P        , "syntax error; expected `%t' before `%t'"                      )
xx(PARSE_SKIPTOK,     N|P        , "%P skipped including one here"                                )
xx(PARSE_MANYBID,       P  |W    , "too many identifiers in a block"                              )
xx(PARSE_MANYBIDSTD,  N    |A|B|C, "ISO C guarantees only %d local identifiers"                   )
xx(PARSE_CLSFIRST,      P  |A|B|C, "ISO C recommends `%t' come first in declaration"              )
xx(PARSE_CLS,         E|P        , "storage-class specifier is not allowed"                       )
xx(PARSE_INVUSE,      E|P        , "invalid use of `%t' in declaration"                           )
xx(PARSE_DEFINT,        P  |W    , "type defaults to `int'"                                       )
xx(PARSE_DEFINTSTD,   N      |B|C, "ISO C forbids implicit `int' since C99"                       )
xx(PARSE_INVCLS,      E|P        , "invalid storage class `%t'"                                   )
xx(PARSE_INVCLSID,    E|P        , "invalid storage class `%t' for %C%i"                          )
xx(PARSE_REDECL1,     E|P        , "redeclaration of%I declared at %p"                            )
xx(PARSE_REDECL2,       P        , "declaration of%I does not match one at %p"                    )
xx(PARSE_HIDEID,        P  |W    , "declaration of%I hides one declared at %p"                    )
xx(PARSE_NOINIT,      E|P        , "initializer is not allowed for %s"                            )
xx(PARSE_VOIDID,      E|P        , "invalid parameter type `void'"                                )
xx(PARSE_VOIDALONE,   E|P        , "`void' must be the only parameter"                            )
xx(PARSE_ELLSEEN,     E|P        , "`...' must be the last in parameters"                         )
xx(PARSE_ELLALONE,    E|P        , "missing named parameter before `...'"                         )
xx(PARSE_QUALVOID,    E|P        , "`void' as only parameter must not be qualified"               )
xx(PARSE_NOPTYPE,     E|P        , "missing parameter type"                                       )
xx(PARSE_PARAMID,     E|P        , "expecting a parameter identifier"                             )
xx(PARSE_EXTRAID,     E|P        , "extraneous identifier%i"                                      )
xx(PARSE_INVARRSIZE,  E|P        , "array size must be greater than 0; adjusted to 1"             )
xx(PARSE_REDEF,       E|P        , "redefinition of%I defined at %p"                              )
xx(PARSE_INVLINK,     E|P        , "inconsistent linkage with%I declared at %p"                   )
xx(PARSE_INVLINKW,      P        , "inconsistent linkage with%I declared at %p"                   )
xx(PARSE_MANYEID,       P  |W    , "too many external-linkage identifiers"                        )
xx(PARSE_MANYEIDSTD,  N    |A|B|C, "ISO C guarantees at most %d external identifiers"             )
xx(PARSE_INCOMPTYPE,  E|P        , "size must be known to define%I"                               )
xx(PARSE_NODECLSPEC,    P  |W    , "missing declaration specifier"                                )
xx(PARSE_EMPTYDECL,   E|P        , "empty declaration"                                            )
xx(PARSE_DECLPARAM,     P        , "declaration must include declarator for parameter"            )
xx(PARSE_NOUSECLS,      P  |W    , "useless storage-class specifier `%t'"                         )
xx(PARSE_INVDECL,     E|P        , "invalid declaration"                                          )
xx(PARSE_INVDCLSTMT,  E|P        , "invalid declaration or statement"                             )
xx(PARSE_INVTYPE,     E|P        , "invalid type specifier (`%t' and `%t')"                       )
xx(PARSE_TYPEDEFF,    E|P        , "function definition declared `typedef'"                       )
xx(PARSE_EXTRAPARAM,  E|P        , "extraneous old-style parameter list"                          )
xx(PARSE_NOPROTO,       P  |W    , "missing prototype"                                            )
xx(PARSE_NOID,        E|P        , "missing identifier"                                           )
xx(PARSE_MANYPARAM,     P  |W    , "too many parameters"                                          )
xx(PARSE_MANYPSTD,    N    |A|B|C, "ISO C guarantees at most %d parameters"                       )
xx(PARSE_INCOMPRET,   E|P        , "size must be known for return type"                           )
xx(PARSE_NOPARAM,     E|P        , "declared parameter%I is missing from the identifier list"     )
xx(PARSE_PARAMMATCH,  E|P        , "parameter mismatch to prototype declared at %p"               )
xx(PARSE_NOPARAMID,   E|P        , "missing name for %o parameter"                                )
xx(PARSE_INCOMPARAM,  E|P        , "size must be known for parameter%I"                           )
xx(PARSE_ENUMID,      E|P        , "expecting an enumerator identifier"                           )
xx(PARSE_ENUMOVER,    E|P        , "overflow in value for enum constant%i"                        )
xx(PARSE_MANYEC,        P  |W    , "too many enum constants in an enum type"                      )
xx(PARSE_MANYECSTD,   N    |A|B|C, "ISO C guarantees at most %d enum constants"                   )
xx(PARSE_ENUMCOMMA,     P  |A    , "C90 disallows extraneous comma at enumerator list"            )
xx(PARSE_ENUMSEMIC,   E|P        , "enumerator definitions must not end with `;'"                 )
xx(PARSE_INVENUM,     E|P        , "unknown enum type"                                            )
xx(PARSE_INVBITTYPE,  E|P        , "bit-fields must have `(signed/unsigned) int' type"            )
xx(PARSE_INVBITSIZE,  E|P        , "illegal bit-field size (that must be [0, %d])"                )
xx(PARSE_NOFNAME,     E|P        , "missing member name"                                          )
xx(PARSE_INCOMPMEM,   E|P        , "size must be known for member"                                )
xx(PARSE_INVFTYPE,    E|P        , "function cannot be a member"                                  )
xx(PARSE_MANYMBR,       P  |W    , "too many struct/union members"                                )
xx(PARSE_MANYMBRSTD,  N    |A|B|C, "ISO C guarantees at most %d members"                          )
xx(PARSE_ANONYTAG,      P      |C, "anonymous struct/union must have no tag"                      )
xx(PARSE_INVFIELD,    E|P        , "invalid %t member declaration"                                )
xx(PARSE_NOFIELD,     E|P        , "missing %t member declaration"                                )
xx(PARSE_NOTAG,       E|P        , "missing %t tag"                                               )
xx(PARSE_REFSTATIC,     P  |W    , "static%I defined but not referenced"                          )
xx(PARSE_REFPARAM,      P  |W    , "parameter%I defined but not referenced"                       )
xx(PARSE_REFLOCAL,      P  |W    , "local%I defined but not referenced"                           )
xx(PARSE_SETNOREFS,     P  |W    , "static%I set but not meaningfully used"                       )
xx(PARSE_SETNOREFP,     P  |W    , "parameter%I set but not meaningfully used"                    )
xx(PARSE_SETNOREFL,     P  |W    , "local%I set but not meaningfully used"                        )
xx(PARSE_UNDSTATIC,   E|P        , "static%I used but not defined in this translation unit"       )
xx(PARSE_ENUMINT,       P  |W    , "`enum' may not be compatible with `int' (see %p)"             )
xx(PARSE_INITCONST,   E|P        , "initializer must be constant"                                 )
xx(PARSE_INVINIT,     E|P        , "invalid initializer; %y given for %y"                         )
xx(PARSE_BIGFLDINIT,    P        , "initializer exceeds bit-field"                                )
xx(PARSE_INCOMINIT,   E|P        , "incomplete type %y cannot be initialized"                     )
xx(PARSE_NOBRACE,     E|P        , "missing `{' for initializer of %y"                            )
xx(PARSE_EXTRABRACE,    P        , "extra brace for scalar initializer"                           )
xx(PARSE_MANYINIT,    E|P        , "too many initializer for %y"                                  )
xx(PARSE_INVMBINIT,   E|P        , "character array initialized from wide string"                 )
xx(PARSE_INVWINIT,    E|P        , "wide character array initialized from non-wide string"        )
xx(PARSE_MANYPD,        P  |W    , "too many nesting levels of parenthesized declarators"         )
xx(PARSE_MANYPDSTD,   N    |A|B|C, "ISO C guarantees at most %d levels"                           )
xx(PARSE_MANYPE,        P  |W    , "too many nesting levels of parenthesized expressions"         )
xx(PARSE_MANYPESTD,   N    |A|B|C, "ISO C guarantees at most %d levels"                           )
xx(PARSE_MANYSTR,       P  |W    , "too many nesting levels of struct/union definitions"          )
xx(PARSE_MANYSTRSTD,  N    |A|B|C, "ISO C guarantees at most %d levels"                           )
xx(PARSE_MIXDCLSTMT,  E|P        , "mixing declarations and statements is not allowed in C90"     )
xx(PARSE_UNUSEDINIT,  E|P        , "useless initializer; nothing to initialize"                   )
xx(PARSE_ATAGPARAM,     P  |W    , "anonymous %t declared in parameter list"                      )
xx(PARSE_PINTFLD,       P  |W    , "signedness of plain bit-field is implementation-defined"      )
xx(PARSE_MIXPROTO,    E|P        , "mixed prototype; former parameters ignored"                   )
xx(PARSE_QUALFRET,      P  |W    , "type qualifier is useless on function return type"            )
xx(PARSE_VOIDOBJ,       P        , "object%I referenced but cannot be defined"                    )
xx(PARSE_MANYDECL,      P  |W    , "too many type derivations in a declarator"                    )
xx(PARSE_MANYDECLSTD, N    |A|B|C, "ISO C guarantees at most %d derivations"                      )

xx(EXPR_SKIPREF,        P        , "reference to incomplete type elided"                          )
xx(EXPR_SKIPVOLREF,     P        , "reference to volatile elided"                                 )
xx(EXPR_SIZEOFINV,    E|P        , "invalid operand given to sizeof"                              )
xx(EXPR_SIZEOFBIT,    E|P        , "bit-field given to sizeof"                                    )
xx(EXPR_PTRINT,         P  |A|B|C, "conversion between pointer and integer is not portable"       )
xx(EXPR_FPTROPTR,       P  |W    , "conversion between function/object pointers is not portable"  )
xx(EXPR_INVCAST,      E|P        , "conversion from %y to %y is not allowed"                      )
xx(EXPR_NOID,         E|P        , "undeclared identifier%I"                                      )
xx(EXPR_IMPLDECL,       P  |W    , "implicit declaration of a function"                           )
xx(EXPR_IMPLDECLSTD,  N      |B|C, "ISO C forbids implicit declaration since C99"                 )
xx(EXPR_ILLTYPEDEF,   E|P        , "illegal use of type name%I"                                   )
xx(EXPR_ILLEXPR,      E|P        , "invalid expression"                                           )
xx(EXPR_NEEDLVALUE,   E|P        , "lvalue required"                                              )
xx(EXPR_VOIDLVALUE,     P        , "`%s' used as an lvalue"                                       )
xx(EXPR_ADDRREG,      E|P        , "taking address of register is not allowed"                    )
xx(EXPR_ATOPREG,      E|P        , "conversion to pointer of register array is not allowed"       )
yy(EXPR_NLVALARR,       P|T|A    , "non-lvalue array does not decay to pointer in C90"            )
xx(EXPR_NEGUNSIGNED,    P  |W    , "unsigned operand of unary -"                                  )
xx(EXPR_ASGNCONST,    E|P        , "assigning to const%I is not allowed"                          )
xx(EXPR_CONDTYPE,     E|P        , "%s%s has illegal type %y; assumed true"                       )
xx(EXPR_NOFUNC,       E|P        , "function or function pointer required"                        )
xx(EXPR_NOMEMBER,     E|P        , "member name expected"                                         )
xx(EXPR_NOSTRUCT1,    E|P        , "struct or union required but `%C' given"                      )
xx(EXPR_NOSTRUCT2,    E|P        , "struct or union pointer required but `%C' given"              )
xx(EXPR_UNKNOWNMEM,   E|P        , "unknown member name%i"                                        )
xx(EXPR_RETINCOMP,    E|P        , "function returns an incomplete type, %y"                      )
xx(EXPR_ARGNOTMATCH,  E|P        , "type error in %o argument to %f; %y given for %y"             )
xx(EXPR_INCOMPARG,    E|P        , "type error in %o argument to %f; %y is an incomplete type"    )
xx(EXPR_POINTER,      E|P        , "pointer required but `%C' given"                              )
xx(EXPR_EXTRAARG,     E|P        , "extra arguments to %f"                                        )
xx(EXPR_MANYARG,        P  |W    , "too many arguments to %f"                                     )
xx(EXPR_MANYARGSTD,   N    |A|B|C, "ISO C guarantees at most %d arguments"                        )
xx(EXPR_INSUFFARG,    E|P        , "insufficient number of arguments to %f"                       )
xx(EXPR_UNKNOWNSIZE,  E|P        , "unknown size for type %y"                                     )
xx(EXPR_ASGNENUMPTR,    P  |W    , "assignment between %y and %y is not portable"                 )
xx(EXPR_ASGNINCOMP,   E|P        , "assignment of incomplete type is not allowed"                 )
xx(EXPR_NEEDOBJ,      E|P        , "addressable object required"                                  )
xx(EXPR_BINOPERR,     E|P        , "operands of %s have illegal types %y and %y"                  )
xx(EXPR_UNIOPERR,     E|P        , "operand of unary %s has illegal type %y"                      )
xx(EXPR_OVFCONSTFP,     P        , "overflow in floating constant expression; not folded"         )
xx(EXPR_OVFCONST,       P        , "overflow in constant expression"                              )
xx(EXPR_OVFCONV,        P        , "overflow in converting constant expression from %y to %y"     )
xx(EXPR_NOINTCONST,   E|P        , "integer constant expression reqruied for %s"                  )
xx(EXPR_NOINTCONSTW,    P  |W    , "integer constant expression reqruied for %s"                  )
xx(EXPR_INVINITCE,      P  |A|B|C, "non-portable constant expression for initializer"             )
xx(EXPR_LARGEVAL,     E|P        , "too large value for %s; adjusted to %d"                       )
xx(EXPR_OVERSHIFT,      P  |W    , "shift results in undefined behavior"                          )
xx(EXPR_OVERSHIFTS,     P  |W    , "shift by %d is undefined"                                     )
xx(EXPR_OVERSHIFTU,     P  |W    , "shift by %u is undefined"                                     )
xx(EXPR_LSHIFTNEG,      P  |W    , "left shift of negative value is undefined"                    )
xx(EXPR_RSHIFTNEG,      P  |W    , "right shift of negative value is not portable"                )
xx(EXPR_DIVBYZERO,      P        , "divide by zero"                                               )
xx(EXPR_UNSIGNEDCMP,    P  |W    , "comparison always results in %s"                              )
yy(EXPR_SYMBOLTRUE,     P  |W    , "address of%I always results in true"                          )
xx(EXPR_NOEFFECT,       P  |W    , "expression always results in %d"                              )
xx(EXPR_VOIDLVALUE1,    P  |A    , "lvalue required but `void' is not an lvalue"                  )
xx(EXPR_VOIDLVALUE2,    P        , "lvalue required but `void' is not an lvalue"                  )
xx(EXPR_BIGFLD,         P        , "value exceeds bit-field"                                      )
xx(EXPR_UNINITREF,      P  |W    , "uninitialized reference to%i"                                 )
xx(EXPR_VALNOTUSED,     P        , "expression result not used"                                   )
xx(EXPR_CHARSUBSCR,     P  |W    , "array subscript has `char' type that might be signed"         )

xx(TYPE_ARRFUNC,      E|P        , "`array of functions' is not allowed"                          )
xx(TYPE_ARRINCOMP,    E|P        , "`array of incomplete type' is not allowed"                    )
xx(TYPE_ARRVOID,      E|P        , "`array of void' is not allowed"                               )
xx(TYPE_BIGARR,       E|P        , "array is too big; size adjusted to %d"                        )
xx(TYPE_BIGARRSTD,    N    |A|B|C, "ISO C guarantees at most %u-byte object"                      )
xx(TYPE_QUALFUNC,       P        , "applying `%t' to function is not allowed; ignored"            )
xx(TYPE_DUPQUAL,        P  |A    , "duplicate qualifier `%t'"                                     )
xx(TYPE_DUPQUALDCLR,    P  |A    , "pointer declarator has duplicate qualifier `%t'"              )
xx(TYPE_FUNCARR,      E|P        , "`function returning array' is not allowed"                    )
xx(TYPE_FUNCFUNC,     E|P        , "`function returning function' is not allowed"                 )
xx(TYPE_STRREDEF,     E|P        , "redefinition of%I previously defined at %p"                   )
xx(TYPE_DIFFTAG,      E|P        , "different kind of%I declared at %p"                           )
xx(TYPE_STRDUPMEM,    E|P        , "duplicate member name%i with one at %p"                       )
xx(TYPE_STRAMBMEM,    E|P        , "ambiguous member%i with one at %p in %y"                      )
xx(TYPE_INVENUM,      E|P        , "unknown enum type"                                            )
xx(TYPE_ERRPROTO,       P        , "erroneous prototype generated for%I due to unnamed tag"       )
xx(TYPE_BIGOBJ,         P        , "size of a type is too big"                                    )
xx(TYPE_BIGOBJADJ,    E|P        , "size of a type is too big; size adjusted to %d"               )

xx(STMT_INFLOOP,        P  |W    , "infinite loop detected"                                       )
xx(STMT_HUGETABLE,      P        , "switch statement generates a huge jump table"                 )
xx(STMT_SWTCHNOINT,   E|P        , "integer required for switch statement"                        )
xx(STMT_SWTCHNOCASE,  E|P        , "switch statement has no cases"                                )
xx(STMT_DUPCASES,     E|P        , "duplicate case label `%d'"                                    )
xx(STMT_DUPCASEU,     E|P        , "duplicate case label `%u'"                                    )
xx(STMT_MANYCASE,       P  |W    , "too many cases in a switch statement"                         )
xx(STMT_MANYCASESTD,  N    |A|B|C, "ISO C guarantees only %d cases"                               )
xx(STMT_DUPLABEL,     E|P        , "redefinition of label%I defined at %p"                        )
xx(STMT_ILLRETTYPE,   E|P        , "illegal return type; %y given for %y"                         )
xx(STMT_RETLOCAL,       P        , "return value depends on address of %s%I"                      )
yy(STMT_UNREACHABLE,    P  |U    , "unreachable code"                                             )
xx(STMT_MANYNEST,       P  |W    , "too many levels of nested statements"                         )
xx(STMT_MANYNESTSTD,  N    |A|B|C, "ISO C guarantees only %d levels"                              )
xx(STMT_ILLBREAK,     E|P        , "illegal break statement"                                      )
xx(STMT_ILLCONTINUE,  E|P        , "illegal continue statement"                                   )
xx(STMT_INVCASE,      E|P        , "case label appears outside switch statement"                  )
xx(STMT_CASENOCONST,  E|P        , "integer constant expression required for case label"          )
xx(STMT_INVDEFAULT,   E|P        , "default label appears outside switch statement"               )
xx(STMT_DUPDEFAULT,   E|P        , "extraneous default label in a switch statement"               )
xx(STMT_EXTRARETURN,  E|P        , "extraneous return value"                                      )
xx(STMT_NORETURN,       P        , "missing return value"                                         )
xx(STMT_GOTONOLAB,    E|P        , "missing label in goto"                                        )
xx(STMT_ILLSTMT,      E|P        , "unrecognized statement"                                       )
xx(STMT_STMTREQ,      E|P        , "statement required before `%t'"                               )
xx(STMT_UNDEFLAB,     E|P        , "undefined label%I"                                            )
xx(STMT_UNUSEDLAB,      P  |W    , "label%I defined but not used"                                 )
xx(STMT_LABELSTMT,    E|P        , "label must have a statement follow it"                        )
xx(STMT_AMBELSE,        P  |W    , "ambiguous `else' can be avoided with braces for `if'"         )
xx(STMT_EMPTYBODY,      P  |W    , "empty body to %s `%s' statement can be misleading"            )
#endif    /* !SEA_CANARY */

xx(XTRA_ERRLIMIT,     E    |F    , "too many errors; compilation stopped"                         )
xx(XTRA_ONCEFILE,     N    |W    , "this is reported only once per file"                          )
xx(XTRA_INVMAIN,        P  |A|B|C, "%D is a non-standard definition"                              )

#undef xx
#undef yy

/* end of xerror.h */
