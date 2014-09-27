beluga: a standard C compiler
=============================

`beluga` is a standard C compiler being developed based on
[`lcc`](https://sites.google.com/site/lccretargetablecompiler/). It supports
C90 (to be precise,
[ISO/IEC 9899:1990](http://www.iso.org/iso/iso_catalogue/catalogue_tc/catalogue_detail.htm?csnumber=17782))
as its ancestor does and is planned to extend the coverage to
[C99](http://www.iso.org/iso/iso_catalogue/catalogue_tc/catalogue_detail.htm?csnumber=29237)
(and
[C11](http://www.iso.org/iso/home/store/catalogue_ics/catalogue_detail_ics.htm?csnumber=57853)
finally).

I'm redesigning each part of the compiler aiming for high configurability; for
example, it would be possible to compile code with 16-bit `int`, unsigned
_plain_ `char` and logical right shift of negative values on a common 32-bit
environment. After finishing the redesign and integrating its front-end with a
back-end, I'll publish its full source code to the public on this page.

An accompanying standard C preprocessor `sea-canary` is carefully designed and
implemented from scratch to support `beluga`. It is fairly fast (even if it has
no optimization for
[macro guard or `#include` guard](http://en.wikipedia.org/wiki/Include_guard)
yet), is correct enough to pass many complicated test cases, produces highly
compact output and has rich diagnostics. For example, it catches code that
subtly depends on an unspecified evaluation order of the `##` operator like
this:

    #define concat(x, y, z) x ## y ## z
    concat(3.14e, -, f)    /* non-portable */

and cleverly keeps track of macro expansions whose recursiveness is determined
by an implementation rather than by the standard:

    #define f(a) a*g
    #define g(a) f(a)
    f(2)(9)    /* non-portable */

(see the defect report
[#017](http://www.open-std.org/Jtc1/sc22/wg14/www/docs/dr_017.html) for the
latter example). The current version of `sea-canary` supports C90, thus some
features like [variadic macros](http://en.wikipedia.org/wiki/Variadic_macro)
introduced in C99 and widely used now are not supported yet.

You can try them out on the `beluga`'s
[web page](http://code.woong.org/beluga).

`INSTALL.md` explains how to build and install the libraries. For the copyright
issues, see the accompanying `LICENSE.md` file.

If you have a question or suggestion, do not hesitate to contact me via email
(woong.jun at gmail.com) or web (http://code.woong.org/).
