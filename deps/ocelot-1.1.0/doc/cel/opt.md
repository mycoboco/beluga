C environment library: option
=============================

This document specifies the option library which belongs to C environment
library.


## 1. Introduction

This library is intended to implement all features of Linux's
[`getopt()` and `getopt_long()`](http://man7.org/linux/man-pages/man3/getopt.3.html)
in an integrated and more consistent fashion; the funtionality of `getopt()`
specified by [POSIX](http://en.wikipedia.org/wiki/POSIX) is also subsumed by
the library.

Precisely, this library:
- supports three ordering modes - argument permutation mode, POSIX-compliant
  mode and _return-in-order_ mode (see below);
- supports a mode in which unrecognized options are preserved with operands for
  separate handling (see "Handling unknown options" section of `opt_t`);
- allows multiple scans of possibly multiple sets of program arguments;
- preserves the original program arguments in its original order;
- supports optional long-named options;
- supports optional short-named options; and
- supports abbreviated names for long-named options.

Suppose that a program supports three long-named options `--html`,
`--html-false` and `--html-true`. For various incomplete options given, the
library behaves as intuitively as possible, for example, `--html-f` is
considered `--html-false`, `--html` is recognized as it is and `--html-`
results in a warning for its ambiguity. This feature is called _abbreviated
names for long-named options_.

This library reserves identifiers starting with `opt_` and `OPT_`, and imports
no other libraries except for the standard ones.


### 1.1. Concepts

There are several concepts used to specify the option library.

_Program arguments_ or _arguments_ for brevity refer to anything given to a
program being executed.

_Operands_ refer to arguments not starting with a hyphen character or to `-`
that denotes the standard input stream. These are sometimes referred to as
_non-option arguments_.

_Options_ refer to arguments starting with a hyphen character but excluding
`-`. _Short-named options_ are options that start with a single hyphen and have
a single character following as in `-x`; several short-named options can be
grouped after a single hyphen as in `-xyz` which is equivalent to `-x -y -z`.
_Long-named options_ are options that start with two hyphens and have a
non-empty character sequence following; for example, `--long-option`.

If an option takes an additional argument which may immediately follow
(possibly with an intervening equal sign) or appear as a separate argument, the
argument is called an _option-argument_. For long-named options,
option-arguments must follow an equal sign unless they appear as separate ones.
(See IEEE Std 1003.1, 2004 Edition,
[12. Utility Conventions](http://pubs.opengroup.org/onlinepubs/009696899/basedefs/xbd_chap12.html).)

_Note that, if an option takes an option-argument that is negative thus starts
with a minus sign, the argument cannot be a separate one, because the separate
argument is to be recognized as another option._

An _option description table_ is an array that has a sequence of options to
recognize and their properties.


### 1.2. How to use the library

Most programs parse program options in very similar ways. A typical way to
handle options is given in the boilerplate code below. You can simply copy it
and modify to add options to the option description table and `case` labels.

The storage used to parse program arguments is managed by the library.


### 1.3. Ordering modes

By default, the library processes options and operands as if they were
permuted so that operands always follow options. That is, the following two
invocations of `util` (where no options take option-arguments) are equivalent
(i.e., program cannot tell the difference):

    util -a -b inputfile outputfile
    util inputfile -a outputfile -b

This behavior canned _argument permutation_, in most cases, helps users to
flexibly place options among operands. Some programs, however, require options
always proceed operands; for example, given the following line,

    util -a util2 -b

it might be wanted to interpret this as giving `-a` to `util` but `-b` to
`util2` which cannot be achieved with argument permutation. For such a case,
this library provides two modes to keep the order in which options and operands
are given: the POSIX-compliant mode and the _return-in-order_ mode which are
denoted by `REQUIRE_ORDER` and `RETURN_IN_ORDER` in a typical implementation of
`getopt()`.

In the POSIX-compliant mode, parsing options stops immediately whenever an
operand is encountered. This behavior is what POSIX requires, as its name
implies.

In the _return-in-order_ mode, encountering operands makes the character valued
`1` returned as if the operand is an option-argument for the option whose short
name has the value `1`.

This ordering mode can be controlled by marking a desired ordering mode in an
option description table or setting an environment variable (see `opt_t`).


### 1.4. Option description tables

An option description table specifies what options should be recognized with
their long and short names and what should be done when encountering them, for
example, whether an additional option-argument is taken and what its type is,
or whether a flag is set and what should be stored into it. Including the
ordering mode, all behaviors of the library can be controlled by the table. See
`opt_t` for more detailed explanation.


### 1.5. Boilerplate code

Using the library starts with invoking `opt_init()`. It takes an option
description table, pointers to parameters of `main()`, a pointer to an object
to which additional information goes during parsing arguments, a default
program name used when no program name is available and a directory separator.
If succeeds, it returns a program name; it can be used to issue messages.

After the library initialized, `opt_parse()` inspects each program argument and
performs what specified by the option description table. In most cases, this
process is made up of a loop containing jumps based on the return value of
`opt_parse()`.

When it is necessary to compare a string argument (that is from an argument of
the `OPT_TYPE_STR` type) with a set of strings to set a variable to an integral
value depending on the string, `opt_val()` would help in most cases; see a
commented-out example in `opt.c`.

As `opt_prase()` reports that all options have been inspected, a program is
granted an access to remaining non-option arguments; or with unrecognized
options if you choose to do. These operands (and options) are inspected as if
they were only arguments to the program.

Because `opt_init()` allocates storages for duplicating pointers to program
arguments, `opt_free()` should be invoked in order to avoid memory leakage
after handling operands has finished.

`opt_abort()` is a function that stops recognition of options being performed
by `opt_parse()`. All remaining options are regarded as operands. It is useful
when a program introduces an option stopper like `--` for its own purposes. It
is preferable for a long-named option to trigger `opt_abort()` because
short-named options can cause a problem when appearing in group.

`opt.c` contains an example designed to use as many facilities of the library
as possible and a boilerplate code that is a simplified version of the example
is given here:

    static struct {
        const char *prgname;
        int verbose;
        int connect;
        /* ... */
    } option;

    int main(int argc, char *argv[])
    {
        static opt_t tab[] = {
            "verbose", 0,           &(option.verbose), 1,
            "add",     'a',         OPT_ARG_NO,        OPT_TYPE_NO,
            "create",  'c',         OPT_ARG_REQ,       OPT_TYPE_STR,
            "number",  'n',         OPT_ARG_OPT,       OPT_TYPE_REAL,
            "connect", UCHAR_MAX+1, OPT_ARG_REQ,       OPT_TYPE_STR,
            "help",    UCHAR_MAX+2, OPT_ARG_NO,        OPT_TYPE_NO,
            NULL,
        }

        option.prgname = opt_init(tab, &argc, &argv, &argptr, PRGNAME, '/');
        if (!option.prgname) {
            opt_free();
            fprintf(stderr, "%s: failed to parse options\n", PRGNAME);
            return EXIT_FAILURE;
        }

        while ((c = opt_parse()) != -1) {
            switch(c) {
                case 'a':
                    /* ... --add or -a given ... */
                    break;
                case 'c':
                    printf("%s: option -c given with value '%s'\n, option.prgname,
                           (const char *)argptr);
                    break;
                case UCHAR_MAX+1:
                    {
                        opt_val_t t[] = {
                            "stdin",   0, "standard input",  0,
                            "stdout",  1, "standard output", 1,
                            "stderr",  2, "standard error",  2,
                            NULL,     -1
                        };
                        option.connect =
                            opt_val(t, argptr, OPT_CMP_CASEIN | OPT_CMP_NORMSPC);
                        if (option.connect == -1) {
                            printf("%s: `stdin', `stdout' or `stderr' must be given for "
                                   "--connect\n", option.prgname);
                            opt_free();
                            return EXIT_FAILURE;
                        }
                    }
                    break;
                case 'n':
                    printf("%s: option -n given", option.prgname);
                    if (argptr)
                        printf(" with value '%f'\n", *(const double *)argptr);
                    else
                        putchar('\n');
                    break;
                case UCHAR_MAX+2:
                    printf("%s: option --help given\n", option.prgname);
                    opt_free();
                    return 0;

                case 0:
                    break;
                case '?':
                case '-':
                case '+':
                case '*':
                    fprintf(stderr, "%s: ", option.prgname);
                    fprintf(stderr, opt_errmsg(c), (const char *)argptr, opt_ambmstr());
                    opt_free();
                    return EXIT_FAILURE;
                default:
                    assert(!"not all options covered -- should never reach here");
                    break;
            }
        }

        if (option.verbose)
            puts("verbose flag is set");
        printf("connect option is set to %d\n", option.connect);

        if (argc > 1) {
            printf("non-option ARGV-arguments:");
            for (i = 1; i < argc; i++)
                printf(" %s", argv[i]);
            putchar('\n');
        }

        opt_free();

        return 0;
    }

The struct object `option` manages all objects set by program arguments. Note
that it has the static storage duration; because its member is used as an
initializer for the option description table that is an array, it has to have
the static storage duration; C99 has removed this restriction.

Each row in the option description table specifies options to recognize:
- `--verbose` has no short name and has a flag variable set to `1` when
  encountered;
- `--add` has a short name `-a` and takes no option-arguments;
- `--create` has a short name `-c` and requires an option-argument of the
  string type;
- `--number` has a short name `-n` and takes an optional option-argument of the
  real type; and
- `--help` has no short name and takes no option-arguments.

The failure of `opt_init()` means memory allocation failed, but you may call
`opt_free()` anyway before terminating the program; it does nothing if no
deallocation is needed.

The `case` labels above one handling `0` are for options given in `tab`. The
labels below them are for exceptional cases and `opt_errmsg()` helps to
construct appropriate messages for them. In addition, there are other ways to
handle those cases; see `opt_errmsg()` for details. Remember that, if invoked,
`opt_free()` should be invoked after all program arguments including non-option
arguments have been processed. Because `opt_init()` makes copies of pointers in
`argv` and `opt_free()` releases storages for them, any access to them gets
invalidated by `opt_free()`.


## 2. APIs

### 2.1. Types

#### `opt_t`

`opt_t` represents an element of an option description table that is used for a
user program to specify options and their properties. An option description
table is an array of `opt_t`, each element of which is consisted of four
members, two of which have overloaded meanings:

| Type           | Name  | Meaning                                                           |
|:--------------:|:-----:|:------------------------------------------------------------------|
| `const char *` | lopt  | long-named option                                                 |
| `int`          | sopt  | short-named option                                                |
| `int *`        | flag  | pointer to flag variable or information about additional argument |
| `int`          | arg   | value for flag variable or type of additional argument            |

- `lopt`: long-named option that can be invoked by two preceding hyphens.
  Optional if a short-named option is given, but it is encouraged to always
  provide a long-named option;
- `sopt`: short-named option that can be invoked by a preceding hyphens.
  Optional if both a long-named option and a flag variable are provided;
- `flag`: if an option does not take an additional argument, `flag` can point
  to an object (called _flag variable_) that is set to the value of `arg` when
  `lopt` or `sopt` option is encountered. If an option can take an additional
  argument, `flag` specifies whether the option-argument is mandatory
  (`OPT_ARG_REQ`) or optional (`OPT_ARG_OPT`); and
- `arg`: if an option does not take an additional argument, `arg` has the value
  to be stored into a flag variable when `lopt` or `sopt` option is
  encountered. If an option can take an additional argument, `arg` specifies
  the type of the option-argument using `OPT_TYPE_BOOL` (option-arguments
  starting with `t`, `T`, `y`, `Y` and `1` means true and others false, `int`),
  `OPT_TYPE_INT` (signed integer, `long`), `OPT_TYPE_UINT` (unsigned integer,
  `unsigned long`), `OPT_TYPE_REAL` (floating-point number, `double`) and
  `OPT_TYPE_STR` (string, `char *`).

To mark the end of the table, `lopt` of the last element has to be set to a
null pointer.

_In earlier versions, flag variables pointed to by `flag` were initialized to
be `0` by `opt_init()`, but this initialization is no longer performed to allow
them to have their own initial values._

For `OPT_TYPE_INT` and `OPT_TYPE_UINT`, the conversion of a given
option-argument recognizes the C-style prefixes; numbers starting with `0` are
treated as octal, and those with `0x` or `0X` as hexadecimal.

Some examples follow:

    static opt_t options[] = {
        { "verbose", 'v', &option_verbose, 1 },
        { "brief",   'b', &option_verbose, 0 },
        { NULL, }
    };

This example says that two options (`--verbose` or `-v`, and `--brief` or
`-b`) are recognized and `option_verbose` is set to `1` when `--verbose` or
`-v` given, and set to `0` when `--brief` or `-b` given.

    static opt_t options[] = {
        "version",  'v',         OPT_ARG_NO, OPT_TYPE_NO,
        "help",     UCHAR_MAX+1, OPT_ARG_NO, OPT_TYPE_NO,
        "morehelp", UCHAR_MAX+2, OPT_ARG_NO, OPT_TYPE_NO,
        NULL,
    };

This example shows options that do not take any additional arguments. Setting
`flag` to a null pointer also says the option takes no argument in which case
the value of `arg` ignored. Thus, the above example can be written as follows
without any change on the behavior:

    static opt_t options[] = {
        "version",  'v',         NULL, 0,
        "help",     UCHAR_MAX+1, NULL, 0,
        "morehelp", UCHAR_MAX+2, NULL, 0,
        NULL,
    };

where you can put any integer in the place of `0`. The former is preferred,
however, because it shows more explicitly the fact that no additional
arguments consumed after each of the options.

When only long-named options need to be provided without introducing flag
variables, values from `UCHAR_MAX+1` to `INT_MAX` (inclusive) can be used for
`sopt`; both macros are from `<limits.h>`.

    static opt_t options[] = {
        "",   'x', OPT_ARG_NO, OPT_TYPE_NO,
        NULL,
    };

On the other hand, providing an empty string for `lopt` as in this example can
specify that an option is only short-named. Note that, however, this is
discouraged; long-named options are much more user-friendly.

    static opt_t options[] = {
        "input", 'i', OPT_ARG_REQ, OPT_TYPE_STR,
        "port",  'p', OPT_ARG_REQ, OPT_TYPE_UINT,
        "start", 's', OPT_ARG_REQ, OPT_TYPE_REAL,
        "end",   'e', OPT_ARG_REQ, OPT_TYPE_REAL,
        NULL,
    };

This example shows options that take additional arguments. `OPT_ARG_REQ` for
`flag` specifies that the option requires an option-argument and that its type
is given in `arg`. For `OPT_TYPE_INT`, `OPT_TYPE_UINT` and `OPT_TYPE_REAL`,
`strtol()`, `strtoul()` and `strtod()` are respectively used to convert
option-arguments.

    static opt_t options[] = {
        "negative", 'n', OPT_ARG_OPT, OPT_TYPE_REAL,
        NULL,
    };

This table specifies the option `--negative` or `-n` takes an optionally given
argument. If an option-argument with the expected form (which is determined by
`strtod()` in this case) follows the option, it is taken. If there is no
argument, or there is an argument but has no expected form, the option works as
if `OPT_ARG_OPT` and `OPT_TYPE_REAL` are replaced by `OPT_ARG_NO` and
`OPT_TYPE_NO`.

The following examples show how to control the ordering mode.

    static opt_t options[] = {
        "+", 0, OPT_ARG_NO, OPT_TYPE_NO,
        /* ... */
        NULL,
    };

Setting the first long-named option to `"+"` or setting the environment
variable named `POSIXLY_CORRECT` says option processing performed by
`opt_parse()` immediately stops whenever an operand encountered (which POSIX
requires).

    static opt_t options[] = {
        "-", 0, OPT_ARG_NO, OPT_TYPE_NO,
        /* ... */
        NULL,
    };

In addition, setting the first long-named option to `"-"` makes `opt_parse()`
returns the character value `1` when encounters an operand as if the operand is
an option-argument for the option whose short name is `'\001'` or `1`. Operands
are not left in `argv` in this mode.

##### Handling unknown options

`opt_parse()` returns `'?'` as an error code when encountering an unknown
option. In some cases, however, it is useful to preserve them in `argv` for
later handling. For example, [`beluga`](http://code.woong.org/beluga/), a C
compiler has multiple back-end implementations and recognizes common options in
the front-end while allowing each back-end target to have its own set of
options which the front-end knows nothing about. By leaving unrecognized
arguments in `argv`, a back-end target has a chance to handle them without help
from the front-end.

Setting the first long-named option to `" "` (a string with a space) forces
`opt_parse()` to silently keep unrecognized options instead of returning an
error code (in this mode, `opt_parse()` never returns `'?'`):

    static opt_t options[] = {
        " ", 0, OPT_ARG_NO, OPT_TYPE_NO,
        /* ... */
        NULL,
    };

If the first long-named option is `"+"` or `"-"` to control the ordering mode,
`" "` must come second:

    static opt_t options[] = {
        "+", 0, OPT_ARG_NO, OPT_TYPE_NO,
        " ", 0, OPT_ARG_NO, OPT_TYPE_NO,
        /* ... */
        NULL,
    };

Due to ambiguities, it is _highly recommended_ that options to recognize later
shall not take option-arguments or shall do by placing them immediatly after
the option or with `=` as in `-Xvalue` and `--option=value`. To explain the
ambiguities, suppose that all uppercase options denote unknown ones. Given
these:

    -X -a foo bar

the first iteration of `opt_parse()` recognizes `-a` and removed it, which
leaves in `argv` these:

    -X foo bar

Even if, in the original sequence, `foo` was an operand, but it may be taken as
an option-argument of `-X` in a rescan. This and similar problems can arise in
all ordering modes when allowing separate option-arguments in a rescan.

Because two consective hyphens(`--`) make the following program argument be
taken as operands even if they start with `-`, `--` is also preserved in
`argv`.

Refer to `opt_reinit()` how to rescan arguments left in `argv`.


#### `opt_val_t`

`opt_val_t` is used to provide for `opt_val()` information that is consisted of
string-integer pairs.

| Type           | Name | Meaning                      |
|:--------------:|:----:|:-----------------------------|
| `const char *` | str  | string to match              |
| `int`          | val  | corresponding integral value |

With the information, `opt_val()` performs a cumbersome job to compare an
`OPT_TYPE_STR` option-argument with a set of strings to set a variable to an
integer value.

    opt_val_t t[] = {
        "ulong",  0, "unsigned long", 0,
        "uint",   1, "unsigned",      1, "unsigned int", 1,
        NULL,    -1
    };

By calling `opt_val()` with `t` defined above and an option-argument (referred
to `argptr` in examples here) of the `OPT_TYPE_STR` type, `opt_val()` returns
`0` if the argument specifies `ulong` or `unsigned long`, and `1` if `uint`,
`unsigned` or `unsigned int`. The array `t` should end with a null pointer and
a value to indicate that none of strings in the array has been matched.

`opt_val()` takes an extra argument `flag` that changes how to compare strings
in it; see `opt_val()` for details.


### 2.2. Constants

#### Types of argument conversions

These `enum` constants represent types of argument conversions:

| Name          | Meaning                                 |
|:-------------:|:----------------------------------------|
| OPT_TYPE_NO   | cannot have type                        |
| OPT_TYPE_BOOL | boolean (`int`) type                    |
| OPT_TYPE_INT  | integer (`long`) type                   |
| OPT_TYPE_UINT | unsigned integer (`unsigned long`) type |
| OPT_TYPE_REAL | floating-point (`double`) type          |
| OPT_TYPE_STR  | string (`char *`) type                  |


#### Controlling `opt_val()`

These `enum` constants controls the behavior of `opt_val()`:

| Name            | Meaning                                    |
|:---------------:|:-------------------------------------------|
| OPT_CMP_NORMSPC | consider `_` and `-` equivalent to a space |
| OPT_CMP_CASEIN  | perform case-insensitive comparison        |


#### Describing option-arguments

These macro constants describes option-arguments:

| Name        | Meaning            |
|:-----------:|:-------------------|
| OPT_ARG_REQ | mandatory argument |
| OPT_ARG_NO  | no argument taken  |
| OPT_ARG_OPT | optional argument  |

See `opt_t` for details.


### 2.3. Objects

#### `const char *opt_ambm[]`

Refer to `opt_parse()` and `opt_ambmstr()` for explanation.


### 2.4. Processing options

#### `const char *opt_init(const opt_t *o, int *pc, char **pv[], const void **pa, const char *name, int sep)`

`opt_init()` prepares to start parsing program arguments.

It takes everything necessary to parse arguments and sets the internal state
properly that is referred to by `opt_parse()` later. It also constructs a more
readable program name by omitting any path preceding the pure name. To do this
job, it takes a directory separator character (`sep`) and a default program
name (`name`) that is used when no name is available from `argv`. A typical use
of `opt_init()` is given at the commented-out example code in `opt.c`.

On success, `opt_init()` returns a program name (non-null pointer). On failure,
it returns a null pointer; `opt_init()` may fail only when allocating
small-sized storage fails, in which case further execution of the program is
very likely to fail due to the same problem.

`opt_init()` can be called again for multiple scans of options, but only after
`opt_free()` has been invoked. Note that, in such a case, only the internal
state and flag variables given with an option description table are
initialized. Other objects probably used to process options in a user code
retain their values, thus should be initialized explicitly by a user code. A
convenient way to handle the initialization is to introduce a structure
grouping all such objects. For example:

    static struct option {
        int html;
        const char *input;
        double val;
        /* ... */
    } option;

where, say, `html` is a flag variable for `--html`, `input` is an argument for
`-i` or `--input`, `val` is an argument for `-n` or `--number`, and so on. By
assigning a properly initialized value to the structure, the initialization can
be readily done as follows.

For C90:

    struct option empty = { 0, };
    option = empty;

For C99:

    option = (struct option){ 0, };

Note that, in this example, the object `option` should have the static storage
duration in order for the `html` member to be given as an initailizer for an
option description table; C99 has no such restriction.

##### May raise

Nothing.

##### Takes

| Name | In/out | Meaning                                                   |
|:----:|:------:|:----------------------------------------------------------|
| o    | in     | option description table                                  |
| pc   | in     | pointer to `argc`                                         |
| pv   | in     | pointer to `argv`                                         |
| pa   | in     | pointer to object to contain argument or erroneous option |
| name | in     | default program name                                      |
| sep  | in     | directory separator (e.g., `/` on Unix-like systems)      |

##### Returns

The program name or a null pointer.


#### `int opt_parse(void)`

`opt_parse()` parses program options.

In typical cases, the caller of `opt_parse()` has to behave based on the result
of `opt_parse()` that is one of:
- `'?'`: unrecognized option. The pointer given through `pa` points to a string
  that represents the option;
- `'-'`: valid option, but no option-argument given even if the option requires
  one, or invalid option-argument given. The pointer given through `pa` points
  to a string that represents the option;
- `'+'`: valid option, but an option-argument given even if the option takes
  none. The pointer given through `pa` points to a string that represents the
  option;
- `'*'`: ambiguous option. It is impossible to identify a unique option with
  the given prefix of a long-named option. Encountering ambiguous options has
  an array of `const char *` named `opt_ambm[]` contain possible matches. This
  information is useful to issue proper messages. A subsequent call to
  `opt_parse()` overwrites `opt_ambm[]`. See also `opt_ambmstr()`;
- `0`: valid option. A given flag variable is set properly, thus nothing for a
  user code to do;
- `-1`: all options have been processed; and
- `1`: (only when the first long-named option is `"-"`; see above) an operand
  is given. The pointer given through `pa` points to the operand.

This means that a valid short-named option cannot be `'?'`, `'-'`, `'+'`,
`'*'`, `-1` or `1` (don't be confused with `'1'`); `0` is allowed to say no
short-named option given when a flag variable is provided; see `opt_t` for
details. In addition, `'='` cannot also be used.

If an option takes an option-argument, the pointer given to `pa` is set to
point to the argument. A subsequent call to `opt_parse()` may overwrite it
unless the type is `OPT_TYPE_STR`.

After `opt_parse()` returns `-1`, `argc` and `argv` (whose addresses are passed
through `pc` and `pv`) are adjusted for a user code to process remaining
operands as if there were no options or option-arguments in program arguments;
see the commented-out example code given in `opt.c`. Once `opt_parse()` starts
the parsing, `argc` and the elements of `argv` are indeterminate, thus an
access to them is not allowed.

_In earlier versions, `opt_parse()` never touched the original contents of
`argv` nor strings pointed to by it. This is not true anymore to remember
unrecognized short-named options in group._

_`opt_init()` has to be invoked successfully before calling `opt_parse()`._

##### May raise

Nothing.

##### Takes

Nothing.

##### Returns

Values explained above.


#### `int opt_val(opt_val_t *tab, const char *s, int flag)`

`opt_val()` helps comparison of a string argument (referred to as `argptr` of
the `OPT_TYPE_STR` type in examples here) with a set of strings.

`opt_val()` compares a string `s` to strings from an `opt_val_t` array as if
done with `strcmp()` except that the case will be ignored if `OPT_CMP_CASEIN`
is set in `flag` and that hyphens(`-`) and underscores(`_`) are regarded as
equivalent to spaces if `OPT_CMP_NORMSPC` set. The later flag is useful when
letting users readily pass to a program an argument with spaces. For example,
when a program accepts `"unsigned int"` as a string option-argument, specifying
`unsigned-int` or `unsigned_int` should be easier than quoting `"unsigned int"`
to preserve the intervening space. That means setting `OPT_CMP_NORMSPC`
effectively changes

    opt_val_t t[] = {
        "unsigned int", 0,
        NULL, -1
    };

to

    opt_val_t t[] = {
        "unsigned int", 0, "unsigned-int", 0, "unsigned_int", 0,
        NULL, -1
    };

If there is a matched string in `tab`, `opt_val()` returns an integer that is
paired with the string, or an integer that is paired with a null pointer
otherwise.

##### May raise

Nothing.

##### Takes

| Name | In/out | Meaning                                     |
|:----:|:------:|:--------------------------------------------|
| tab  | in     | array containing string-integer pairs       |
| s    | in     | string argument to compare with `tab`       |
| flag | in     | flag to control the behavior of `opt_val()` |

##### Returns

An integer corresponding to a matched string.


#### `void opt_abort(void)`

`opt_abort()` aborts parsing options immediately handling the remaining
arguments as operands.

Having invoked `opt_abort()`, `opt_parse()` need not be called to access to
operands; `argc` and `argv` are properly adjusted as if `opt_parse()` has
returned `-1` except that the remaining options (if any) are treated as
operands. If `opt_parse()` invoked after aborting the parsing, `opt_parse()`
does nothing and returns `-1`.

##### May raise

Nothing.

##### Takes

Nothing.

##### Returns

Nothing.


#### `const char *opt_ambmstr(void)`

When `opt_parse()` encounters an ambiguous option, it has a string array
`opt_ambm[]` contain possible matches. `opt_ambm[]` has a fixed size (that is,
not dynamically allocated) and has up to 5 candidates. A diagnostic message can
be constructed to show 4 candidates and to say "and more" or "..." if
`opt_ambm[4]` is not null. (_The size of `opt_ambm[]` may change in a future
release and `sizeof(opt_ambm)/sizeof(opt_ambm[0])` is preferred to `4`, for
example, to refer to the last element._)

`opt_ambmstr()` does that job for you, and returns a string that looks like

    match1, match2, match3, match4, ...

where `...` is literal.

The string buffer used is also statically allocated, so the resulting string
may contain less than 4 candidates if there is no room to contain more. `, ...`
is not inserted when the buffer can hold all possible matches.

A subsequence call to `opt_ambmstr()` overwrites the returned string.

##### May raise

Nothing.

##### Takes

Nothing.

##### Returns

A string to enumerate possible matches.


#### `const char *opt_errmsg(int c)`

Given an error code that is one of `'?'`, `'-'`, `'+'` and `'*'`,
`opt_errmsg()` returns a string that can be used as a format string for the
`printf()` family. A typical way to handle exceptional cases that `opt_parse()`
may return is as follows:

    switch(c) {
        /* ... cases for valid options ... */
        case 0:
            break;
        case '?':
            fprintf(stderr, "%s: unknown option '%s'\n", option.prgname, (const char *)argptr);
            opt_free();
            return EXIT_FAILURE;
        case '-':
            fprintf(stderr, "%s: no or invalid argument given for '%s'\n", option.prgname,
                    (const char *)argptr);
            opt_free();
            return EXIT_FAILURE;
        case '+':
            fprintf(stderr, "%s: option '%s' takes no argument\n", option.prgname,
                    (const char *)argptr);
            opt_free();
            return EXIT_FAILURE;
        case '*':
            fprintf(stderr, "%s: ambiguous option '%s' (%s)\n", option.prgname,
                    (const char *)argptr, opt_ambmstr());
            opt_free();
            return EXIT_FAILURE;
        default:
            assert(!"not all options covered -- should never reach here");
            break;
    }

where `case 0` is for options that sets a flag variable so in most cases leaves
nothing for a user code to do. The following four `case` labels handle
erroneous cases and the `default` case is there to handle what is never
supposed to happen.

As repeating this construct for every program using this library is cumbersome,
for convenience `opt_errmsg()` is provided to handle those four erroneous cases
as follows:

    switch(c) {
        /* ... cases for valid options ... */
        case 0:
            break;
        case '?':
        case '-':
        case '+':
        case '*':
            fprintf(stderr, "%s: ", option.prgname);
            fprintf(stderr, opt_errmsg(c), (const char *)argptr, opt_ambmstr());
            opt_free();
            return EXIT_FAILURE;
        default:
            assert(!"not all options covered -- should never reach here");
            break;
    }

or more compactly:

    switch(c) {
        /* ... cases for valid options ... */
        case 0:
            break;
        default:
            fprintf(stderr, "%s: ", option.prgname);
            fprintf(stderr, opt_errmsg(c), (const char *)argptr, opt_ambmstr());
            opt_free();
            return EXIT_FAILURE;
    }

A string returned by `opt_errmsg()` has two `%s`'s only when `c` is `*` (to
indicate an ambiguous option), and it is not an error to give extra arguments
to `printf()`.

The difference of the last two is that the latter turns the assertion in the
former (that possibly gets _dropped_ from the delivery code) into a defensive
check (that does _not_). Note that the returned format string contains a
newline.

If a user needs flexibility on the format of diagnostics or actions done in
those cases, resort to the cumbersome method shown first.

##### May raise

Nothing.

##### Takes

| Name | In/out | Meaning                           |
|:----:|:------:|:----------------------------------|
| c    | in     | error code `opt_parse()` returned |

##### Returns

A format string for diagnostic message.


#### `void opt_free(void)`

`opt_free()` cleans up any storage allocated by `opt_init()`. It also
initializes the internal state, which allows for multiple scans; see
`opt_init()` for some caveat when scanning options multiple times.

`opt_free()`, if invoked, should be invoked after all arguments including
operands have been processed. Because `opt_init()` makes copies of pointers in
`argv` of `main()`, and `opt_free()` releases storages for them, any access to
them gets invalidated by `opt_free()`.

##### May raise

Nothing.

##### Takes

Nothing.

##### Returns

Nothing.


#### `const char **opt_reinit(const opt_t *o, int *pc, char **pv[], const void **pa)`

See "Handling unknown options" section of `opt_t` for how to leave unknown
options in `argv`.

To let `opt_parse()` rescan options left in `argv`, it is necessary to invoke
`opt_init()` for reinitialization, which requires `opt_free()` before it. Since
`opt_free()` frees storage allocated for `argv`, you are required to make a
copy of `argv` before `opt_free()` as follows:

    const char **p;
    p = malloc(sizeof(*p) * (argc+1));    /* +1 for NULL */
    assert(p);
    memcpy(p, argv, sizeof(*p)*(argc+1));
    opt_free();    /* frees argv */
    argv = p;
    if (opt_init(newtab, &argc, &argv, /* ... */))
        /* handle opt_init()'s failure */
    free(p);    /* opt_init() makes its own copy and sets argv to point to it */
    /* use opt_parse() */
    opt_free();

This is cumbersome, so `opt_reinit()` is provided.

`opt_reinit()` takes arguments similar to `opt_init()`'s and returns the
program name or a null pointer.

    /* no need to call opt_free() nor opt_init() */
    if (!opt_reinit(new_tab, &argc, &argv, &argptr))
        /* handle opt_reinit()'s failure */
    /* use opt_parse() */
    opt_free();

The default program name and a directory seperator are not taken and what have
been delivered through `opt_init()` are used.

##### May raise

Nothing.

##### Takes

| Name | In/out | Meaning                                                   |
|:----:|:------:|:----------------------------------------------------------|
| o    | in     | option description table                                  |
| pc   | in     | pointer to `argc`                                         |
| pv   | in     | pointer to `argv`                                         |
| pa   | in     | pointer to object to contain argument or erroneous option |

##### Returns

The program name or a null pointer.


## 3. Contact me

Visit [`code.woong.org`](http://code.woong.org) to get the latest version of
this library. Any comments about the library are welcomed. If you have a
proposal or question on the library just email me, and I will reply as soon as
possible.


## 4. Copyright

For the copyright issues, see `LICENSE.md`.
