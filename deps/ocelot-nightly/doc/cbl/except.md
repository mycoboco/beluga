C basic library: exception
==========================

This document specifies the exception library which belongs to C basic library.
The basic structure is from David Hanson's book,
[C Interfaces and Implementations](https://sites.google.com/site/cinterfacesimplementations/).
I modified the original implementation to make it more appropriate for my other
projects, to conform to the C standard and to enhance its readability.

The book explains its design and implementation in a very comprehensive way.
Not to mention the copyright issues, the internals of the library is not
explained here. Typical exception-handling constructs and emphasis on crucial
issues and explanations for APIs, however, are given to aid the use of the
library.


## 1. Introduction

The library reserves identifiers starting with `except_` and `EXCEPT_`, and
imports the assertion library.


### 1.1. How to use the library

The following construct shows how a typical TRY-EXCEPT statement looks like.

    EXCEPT_TRY
        S;
    EXCEPT_EXCEPT(e1)
        S1;
    EXCEPT_EXCEPT(e2)
        S2;
    EXCEPT_ELSE
        S3;
    EXCEPT_END;

`EXCEPT_TRY` starts a TRY-EXCEPT or TRY-FINALLY statement. The statements
following `EXCEPT_TRY` (referred to as `S` in this example) are executed, and
if an exception is occurred during that execution, the control moves to one of
EXCEPT clauses with a matching exception or the ELSE clause. The statements
`Sn` under the matched EXCEPT clause or ELSE clause are executed and the
control moves to `EXCEPT_END`.

    EXCEPT_TRY
        S
    EXCEPT_ELSE
        S1
    EXCEPT_END;

A construct without any EXCEPT clause is useful when catching all exceptions
raised during execution of `S` in a uniform way. If other exception handers are
established during execution of `S` only uncaught exceptions there move the
control to the ELSE clause above. For example, any uncaught exception with no
recovery (e.g., assertion failures or memory allocation failures) can be
handled in the `main` function as follows:

    int main(void)
    {
        EXCEPT_TRY
            real_main();
        EXCEPT_ELSE
            fprintf(stderr,
                    "An internal error occurred with no way to recover.\n"
                    "Please report this error to somebody@somewhere.\n\n");
            EXCEPT_RERAISE;
        EXCEPT_END;

        return 0;
    }

A TRY-FINALLY statement looks like:

    EXCEPT_TRY
        S;
    EXCEPT_FINALLY
        S1;
    EXCEPT_END;

The statements following `EXCEPT_FINALLY` are executed regardless of occurrence
of an exception, so if a kind of clean-up like closing open files or freeing
allocated storage is necessary to be performed unconditionally, `S1` is the
right place to put. If the exception caught by a TRY-FINALLY statement needs to
be also handled by a TRY-EXCEPT statement, `EXCEPT_RERAISE` raises it again to
give the previous handler (if any) a chance to handle it.

Note that each group of the statements, say, `S`, `S1` and so on, constitutes
an independent block; opening or closing braces are hidden in `EXCEPT_TRY`,
`EXCEPT_EXCEPT`, `EXCEPT_FINALLY` and `EXCEPT_END`. Therefore variables
declared in a block, say, `S` is not visible to other blocks, say, `S1`.

And even if not explicitly specified in Hanson's book, it is possible to
construct an exception handling statement which has both EXCEPT and FINALLY
clauses, which looks like:

    EXCEPT_TRY
        S;
    EXCEPT_EXCEPT(e)
        Se;
    EXCEPT_FIANLLY
        Sf;
    EXCEPT_END;

Looking into the implementation by combining those macros explains how it
works. Finding when it is useful is up to users.


### 1.2. Caveats

The exception handling mechanism given here is implemented using a non-local
jump provided by [`<setjmp.h>`](http://en.wikipedia.org/wiki/Setjmp.h). Every
restriction applied to `<setjmp.h>` also applies to this library. For example,
there is no guarantee that an updated `auto` variable preserves its last stored
value if the update done between `setjmp()` and `longjmp()`. For example,

    {
        int i;

        EXCEPT_TRY
            i++;
            S;
        EXCEPT_EXCEPT(e1)
            S1;
        EXCEPT_TRY;
    }

If an exception `e1` occurs, which moves the control to `S1`, it is unknown
what the value of `i` is in the EXCEPT clause above. A way to walk around this
problem is to declare `i` as `volatile` or `static`. (See the manpage for
`setjmp()` and `longjmp()`.)

At the first blush, this restriction seems too restrictive, but it is not. The
restriction applies only to those non-volatile variables with automatic storage
duration and belonged to the function containing `EXCEPT_TRY` (which has
`setjmp()` in it), and only when they are modified between `setjmp()`
(`EXCEPT_TRY`) and corresponding `longjmp()` (`EXCEPT_RAISE()`).

One more thing to remember is that the ordinary `return` does not work in the
statements `S` above because it knows nothing about maintaining the exception
stack. Inside `S`, the exception frame has already been pushed to the exception
stack. Returning from it without adjusting the stack by popping the current
frame spoils the exception handling mechanism, which results in undefined
behavior. `EXCEPT_RETURN` is provided for this purpose. It does the same thing
as the ordinary `return` except that it adjusts the exception stack properly.
Also note that `EXCEPT_RETURN` should not be used in a EXCEPT, ELSE or FINALLY
clause; entering those clauses entails popping the current frame from the
stack.

In general, it is said that recovery from an erroneous situation gets easier if
you have a way to handle it with an exception and its handler. In practice,
with the implementation using a non-local jump facility like this library,
however, that is not always true. If a program manages resources like allocated
memory and open file pointers in a complicated fashion and an exception can be
raised at any deeply nested level, it is very likely for you to lose control
over them. In addition to it, keeping internal data structures consistent is
also a problem. If an exception can be triggered during modifying fields of an
internal data structure, it is never a trivia to guarantee consistency of that.
Therefore, an exception handling facility this library provides is in fact best
suited for handling various problematic circumstances in one spot and then
terminating the program's execution almost immediately. If you would like your
code to be tolerant to an exceptional case by, for example, making it revert to
an inferior but reliable approach, you have to keep these facts in your mind.


### 1.3. Improvements

A diagnostic message printed when an assertion is failed changes in C99 to
include the name of a function where the failure occurs. This can be readily
attained with a newly introduced predefined identifier `__func__`. To provide
more useful information, if an implementation claims to support C99 by defining
the macro `__STDC_VERSION__` properly, the library also includes the function
name when making up the message output when an uncaught exception detected. For
the explanation on `__func__` and `__STDC_VERSION__`, see ISO/IEC 9899:1999 or
its later editions.


### 1.4. Boilerplate code

To show a boilerplate code, suppose that a module named `mod` defines and may
raise exceptions named `mod_exceptfail` and `mod_exceptmem`, and that code
invoking the module is expected to install an exception handler for that
exception. Now implementing the module `mod` looks in part like:

    const except_t mod_exceptfail = { "Some operation failed" };
    const except_t mod_exceptmem = { "Memory operation failed" };

    /* ... */

    int mod_oper(int arg)
    {
        /* ... */
        if (!p)
            EXCEPT_RAISE(mod_exceptfail);
        else if (p != q)
            EXCEPT_RAISE(mod_exceptmem);
        /* ... */
    }

where the names of exceptions and the contents of strings given as initializers
are up to an user. Those initializer strings are printed out when the
corresponding exception is raised but not caught by a user-installed handler.
By installing an exception handler with a TRY-EXCEPT construct, code that
invokes `mod_oper()` can handle exceptions `mod_oper()` may raise:

    EXCEPT_TRY
        result = mod_oper(value);
        /* ... */
    EXCEPT_EXCEPT(mod_exceptfail)
        fprintf(stderr, "program: some operation failed; no way to recover\n");
        EXCEPT_RERAISE;
    EXCEPT_EXCEPT(mod_exceptmem)
        fprintf(stderr, "program: memory allocation failed; internal buffer used\n");
        /* prepare internal buffer and retry */
    EXCEPT_END

Note that exceptions other than `mod_exceptfail` and `mod_exceptmem` are
uncaught by this handler and handed to an outer handler if any.


## 2. APIs

### 2.1. Types

#### `except_t`

The uniqueness of an exception is determined by the address of a member in
`except_t`; so make an `except_t` object have static storage duration when
declaring it. It is undefined what happens when using an exception object with
automatic storage duration.


### 2.1. Raising exceptions

There are two function-like macros to raise an exception; `EXCEPT_RAISE()`
raises an exception given as an argument, while `EXCEPT_RERAISE()` re-raises
the most recently raised exception, thus takes no argument.

#### `void EXCEPT_RAISE(except_t e)`

The `EXCEPT_RAISE()` macro raises an exception `e`.

`EXCEPT_RAISE()` is in fact a macro and it takes the address of `e` before
invocation to an actual function. An argument to `e`, therefore, has to be an
lvalue.

##### May raise

A given exception, `e`.

##### Takes

| Name  | In/out | Meaning            |
|:-----:|:------:|:-------------------|
| e     | in     | exception to raise |

##### Returns

Nothing.


#### `void EXCEPT_RERAISE(void)`

`EXCEPT_RERAISE()` macro re-raises the exception that has been raised most
recently.

##### May raise

The exception that has been raised most recently.

##### Returns

Nothing.


### 2.2. TRY-EXCEPT construct

TRY-EXCEPT and TRY-FINALLY statements can be constructed using several macros
that help ordinary C code to look like having them.


#### `EXCEPT_TRY`

The `EXCEPT_TRY` macro starts a TRY statement. Statements (referred to as `S`
hereafter) whose exception is to be handled in EXCEPT, ELSE or FINALLY clause
follow.

_Do not forget using `EXCEPT_RETURN` when returning within `S`. See
`EXCEPT_RETURN` for more details. Besides, the TRY-EXCEPT/FINALLY statement
uses the non-local jump mechanism provided by `<setjmp.h>`, which means any
restriction applied to `<setjmp.h>` also applies to the TRY-EXCEPT/FINALLY
statement._


#### `EXCEPT_EXCEPT(e)`

The `EXCEPT_EXCEPT(e)` macro starts an EXCEPT(e) clause. When an exception `e`
is raised during execution of `S`, statements following `EXCEPT_EXCEPT(e)` are
executed. After finishing execution of those statements, the control moves to
the end of the TRY-EXCEPT statement.


#### `EXCEPT_ELSE`

The `EXCEPT_ELSE` macro starts an ELSE clause. If there is no matched EXCEPT
clause for a raised exception, the control moves to the statements following
the ELSE clause.


#### `EXCEPT_FINALLY`

The `EXCEPT_FINALLY` macro starts a FINALLY clause. It is used to construct a
TRY-FINALLY statement, which is useful when some clean-up is necessary before
exiting the TRY-FINALLY statement; the statements within the FINALLY clause are
executed whether or not an exception occurs. The `EXCEPT_RERAISE` macro can be
used to hand over the not-yet-handled exception to the previous handler.

_Remember that, raising an exception pops up the exception stack and re-raising
an exception in a FINALLY clause has the effect to move the control to the
outer (previous) handler. Also note that, even if not explicitly specified, a
TRY-EXCEPT-FINALLY statement (there are both EXCEPT and FINALLY clauses) is
possible and works as expected._


#### `EXCEPT_END`

The `EXCEPT_END` macro ends a TRY-EXCEPT or TRY-FINALLY statement. If a raised
exception is not handled by the current handler, it will be passed to the
previous handler if any.


### 2.3. Returning within TRY-EXCEPT

#### `EXCEPT_RETURN`

The `EXCEPT_RETURN` macro returns the control to the caller function within a
TRY-CATCH statement.

In order to maintain the stack handling nested exceptions, the ordinary
`return` statement should be avoided in statements following `EXCEPT_TRY`.
Because `return` has no idea about the exception frame, `returning` without
using `EXCEPT_RETURN` from statements following `EXCEPT_TRY` spoils the
exception stack. `EXCEPT_RETURN` adjusts the stack properly by popping the
current exception frame in such a case.

_Note that the current exception frame is popped when an exception occurs
during execution of statements following `EXCEPT_TRY` and before the control
moves to one of EXCEPT, ELSE and FINALLY clauses, which means using
`EXCEPT_RETURN` within those clauses is not allowed because it touches the
previous, not the current, exception frame._


## 3. Future directions

### 3.1. Stack traces

The current implementation provides no information about the execution stack of
a program when an occurred exception leads to abnormal termination. They have
to track function calls by themselves to pinpoint the problem, which is surely
a burden on programmers. Thus, showing stack traces on an uncaught exception
would be useful especially when they include enough information like callers'
names, calling sites and preferably arguments.


## 4. Contact me

Visit [`code.woong.org`](http://code.woong.org) to get the latest version of
this library. Any comments about the library are welcomed. If you have a
proposal or question on the library just email me, and I will reply as soon as
possible.


## 5. Copyright

For the copyright issues, see `LICENSE.md`.
