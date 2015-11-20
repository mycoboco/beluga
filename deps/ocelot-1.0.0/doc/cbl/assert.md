C basic library: assertion
==========================

This document specifies the assertion library which belongs to C basic library.
The basic structure is from David Hanson's book,
[C Interfaces and Implementations](https://sites.google.com/site/cinterfacesimplementations/).
I modified the original implementation to make it more appropriate for my other
projects as well as to increase its readability; conformance is not an issue
here because trying to replace an existing standard library by _knocking it
out_ is already considered non-conforming.


## 1. Introduction

Replacing `<assert.h>` with a version to support exceptions is useful based on
some assumptions:
- deactivating assertions in product code does not bring much benefit to the
  program's performance (only profiling would be able to say how much
  assertions degrades performance);
- avoiding assertions showing cryptic diagnostic messages leads users into a
  more catastrophic situation like dereferencing a null pointer;
- a diagnostic message like `Uncaught exception Assertion failed raised at
  file.c:54` without an expression that caused the assertion failure conveys
  equivalent information than messages from the standard version, when the
  programmer can access to the problematic file; and
- dealing with assertion failures by an exception has an advantage that every
  assertion can be handled in a uniform way (see an example below).

More detailed explanation on the motivation is given in Hanson's book, so I
should stop here, but an introduction to and explanations for APIs are given to
aid the use of the library.

The assertion library reserves identifiers `assert` and `ASSERT`, and imports
the exception library.


### 1.1. How to use the library

Using the assertion Library is not quite different from using the standard
version. Differently from Hanson's original implementation, this implementation
includes the standard version `<assert.h>` if either `NDEBUG` or
`ASSERT_STDC_VER` is `#define`d.

The following example shows how to handle assertion failures in one place:

    #include "assert.h"
    #include "except.h"

    int main(void)
    {
        EXCEPT_TRY
            real_main();
        EXCEPT_EXCEPT(assert_exceptfail)
            fprintf(stderr,
                    "An internal error occurred with no way to recover.\n"
                    "Please report this error to me@example.com.\n\n");
            RERAISE;
        EXCEPT_END;

        return 0;
    }

If you use an `ELSE` clause instead of the `EXCEPT` clause above, any uncaught
exception lets the program print the message before aborting. (See the
documentation of the exception library.)


## 2. APIs

### 2.1. Exceptions

#### `const except_t assert_exceptfail`

This exception is occurred when an assertion described by `assert()` fails


### 2.2. Assertion

#### `void assert(e)`

The macro `assert()` replaces the standard `assert()` to support exceptions. An
activated assert() raises an exception named `assert_exceptfail`. The
differences between this and the standard's version are that the exception
version:
- does not print the scalar expression `e`; and
- does not abort; it merely raises an exception and let the exception handler
  decide what to do.

##### May raise

`assert_exceptfail`.

##### Takes

| Name  | In/out | Meaning                                  |
|:-----:|:------:|:-----------------------------------------|
| e     | in     | scalar expression to check for assertion |

##### Returns

Nothing.



## 3. Contact me

Visit [`code.woong.org`](http://code.woong.org) to get the latest version of
this library. Any comments about the library are welcomed. If you have a
proposal or question on the library just email me, and I will reply as soon as
possible.


## 4. Copyright

For the copyright issues, see `LICENSE.md`.
