beluga: a standard C compiler
=============================

`beluga` is a standard C compiler being developed based on an earlier version
of [`lcc`](https://github.com/drh/lcc). It supports C90 (to be precise,
[ISO/IEC 9899:1990](https://www.iso.org/iso/iso_catalogue/catalogue_tc/catalogue_detail.htm?csnumber=17782))
as its ancestor does and is planned to extend the coverage to
[C99](https://www.iso.org/iso/iso_catalogue/catalogue_tc/catalogue_detail.htm?csnumber=29237)
(and
[C11](https://www.iso.org/iso/home/store/catalogue_ics/catalogue_detail_ics.htm?csnumber=57853)
finally).

Compared to its parent, `beluga` carefully implements the language standard
and thus provides production-quality diagnostics including caret diagnostics,
range highlighting, `typedef` preservation and macro expansion tracking:

![screenshot for enhanced front-end features](https://code.woong.org/common/files/compiler-20171025.png)

The generated code is not highly optimized, but satisfactory enough for daily
use. (_This is a hobby project; never easy for me alone to catch up production
compilers like [gcc](https://gcc.gnu.org/) and
[clang+llvm](https://clang.llvm.org/)_.)

`beluga` currently produces assembly output for
[x86](https://en.wikipedia.org/wiki/X86) only (and uses an assembler from the
target system). Thanks to its origin, however, it can be readily retargeted to
other platforms. Support for 64-bit machines (like
[x86-64](https://en.wikipedia.org/wiki/X86-64)) requires new significant
features to be implemented and is one of most important goals of this project.

Also I'm redesigning each part of the compiler aiming for better structure
(e.g., see below for an integrated preprocessor) and have a plan to completely
replace the back-end interface and implementation to ease adoptation of more
ambitious optimization techniques mainly based on a
[CFG](https://en.wikipedia.org/wiki/Control_flow_graph).


#### An integrated preprocessor

The preprocessor formerly developed as a separate executable under the name of
`sea-canary`, has been integrated into the compiler. It reads source code and
delivers tokens (not characters) to the compiler proper via a token stream (not
via a temporary file). It is
[fairly fast](https://github.com/mycoboco/beluga/issues/4), is correct enough
to pass many complicated
[test cases](https://github.com/mycoboco/beluga/tree/master/tst/cpp), produces
highly compact output and has rich diagnostics. For example, it catches, with
`-Wtoken-paste-order` option, code that subtly depends on an unspecified
evaluation order of the `##` operator like this:

    #define concat(x, y, z) x ## y ## z
    concat(3.14e, -, f)    /* non-portable */

and, due to the line mapper shared by the compiler, it pinpoints problematic
spots as precisely as possible:

![range highlighting on sub-expression from macro expansion](https://code.woong.org/common/files/pp-20171025.png)

The current version conforms to C90, but supports features like empty arguments
and [variadic macros](https://en.wikipedia.org/wiki/Variadic_macro) introduced
in C99 and widely used now.


#### Try it out

You can try them out on the `beluga`'s
[web page](https://code.woong.org/beluga).


#### How to install

`INSTALL.md` explains how to build and install the package. For the copyright
issues, see the accompanying `LICENSE.md` file.


#### Contact

If you have a question or suggestion, do not hesitate to contact me via email
(woong.jun at gmail.com) or web (https://code.woong.org/).
