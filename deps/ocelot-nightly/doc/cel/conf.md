C environment library: configuration
====================================

This document specifies the configuration library which belongs to C
environment library.


## 1. Introduction

This library reads an _ini_-style configuration file and
allows its user to readily access to values set by the file. There is no _de
jure_ standard for _ini_ files, but this library supports most of what the
[Wikipedia page](http://en.wikipedia.org/wiki/INI_file) describes; sections
(with no support for nested ones), line concatenation by a backslash, escape
sequences and so on. Differently from other implementations, this library
supports a simple type system. This aids its users to retrieve values set by a
configuration file without manual conversion to a desired type.

This library reserves identifiers starting with `conf_` and `CONF_`, and
imports the assertion library (which requires the exception library), the
memory library, the hash library and the table library.


### 1.1. Concepts

There are several concepts used to specify the library.

_Configuration variables_ (called simply _variables_ hereafter) are variables
managed by the library and set by a default setting, a configuration file or a
program. Variables have names and types.

A set of variable can be grouped and defined to belong to a _configuration
section_ (called simply _section_ hereafter). A section comprises a distinct
namespace, thus two variables with the same name designate two different
variables if they belong to different sections.

The _global section_ is an unnamed section that always exists. How to designate
the section and when variables belong to it are described below. The _current
section_ is a section set by a user program so that variables with no section
designation are assumed to belong to it; `conf_section()` is used to set the
current section.

The supported _types_ are boolean, signed/unsigned integer, real and string.
The string type is most generic and the library provides facilities to convert
that type to others.

A _configuration description table_ is an array that has a sequence of variable
names and their properties including a default value. The table also specifies
a set of supported sections and variables. If supplied, the library recognizes
only sections and variables appeared in the table. Otherwise, all sections and
variables mentioned in a configuration file are recognized.


### 1.2. How to use the library

The configuration library reads an _ini_-style configuration file and set
variables according to its contents. The library changes its behavior depending
on whether a configuration description table is given by `conf_preset()`. The
table can be composed by creating an array of `conf_t` and passed to the
library. If `conf_preset()` is not invoked, `conf_init()` has to be used to
initiate the library and to read a specified configuration file. If
`conf_init()` is called after `conf_preset()`, the contents read from a
configuration file override what the table sets.

A configuration description table gives a list of sections and variables that
the library can recognize and any other section/variable names that appear in a
configuration file (but not in the table) are treated as an error.

If no configuration description table given, the library recognizes all
possible sections and variables during `conf_init()` reads a configuration
file, and because there is no way to prescribe the type of each variable, all
variables are assumed to have the string type.

The storage used to maintain a program configuration itself is managed by the
library.


### 1.3. Configuration description tables

If ever invoked, `conf_preset()` has to be invoked before `conf_init()`. If a
program need not read a configuration file and uses only predefined settings
given through `conf_preset()`, it need not call `conf_init()` at all. A
configuration description table is an array of `conf_t` and enumerates
section/variable names, their types and default values. For more details
including how to designate a section and variable in the table and what each
field of the table means, see `conf_t`.


### 1.4. Configuration files

A configuration file basically has the form of a so-called _ini_ file. The file
is consisted of variable-value pairs belonged to a certain section as follows:

    [section_1]
    var1 = value1
    var2 = value2

A string between the square brackets specifies a section, and variable-value
pairs appear below are belonged to that section. Names for sections and
variables have to be consisted of only digits, alphabets and an
underscore(`_`). By default, names are case-insensitive (setting the
`CONF_OPT_CASE` bit in the second argument to `conf_preset()` and `conf_init()`
can change this behavior). They cannot have an embedded space. What characters
constitute digits and alphabets depend on the locale where the library is used.
If a program changes its locale to other than "C" locale, characters that are
allowed for section/variable names also change.

If the pairs are given before any section has not been specified, they belong
to the _global_ section. The global section also can be specified by an empty
section name as shown below:

    []    # global section
    var1 = value1

A section does not nest and variables belonging to a section need not be
gathered.

    var0 = value0    # belongs to the global section

    [section_1]
    var1 = value1    # belongs to section_1

    [section_2]
    var2 = value2    # belongs to section_2

    [section_1]
    var3 = value3    # belongs to section_1, now section_1 has two variables

Two different sections have the same variable and they are distinct.

    [section_1]
    var1 = value1    # var1 belonging to section_1

    [section_2]
    var1 = value1    # var1 belonging to section_2

If a variable appears with the same name as one that appeared first under the
same section, the value is overwritten by the latter setting.

    [section_1]
    var1 = value1    # var1 has value1
    var1 = value2    # var2 now has value2

Comments begin with a semicolon(`;`) or a number sign(`#`) and ends with a
newline as you have shown in examples above.

If the last character of a line is a backslash(`\`) without any trailing
spaces, its following line is spliced to the line by eliminating the backslash
and following newline. Any whitespaces preceding the backslash and any leading
whitespaces in the following line are replaced with a space.

    [section_1]
    var1 = val\
    ue             # var1 = value
    var2 = value\
               2      # var2 = value 2
    var3 = value    \
               3         # var3 = value 3

Values following an equal sign(`=`) after variables can have two forms, quoted
and unquoted. Quoted values have to start with either a double-quote(`"`) or
single-quote (`'`) and end with the matching character; that is, the whole
value should be quoted. A semicolon(`;`) or number sign(`#`) in a quoted value
does not start a comment.

    [section_1]
    var1 = "quoted value. ; or # starts no comment"  # now this is comment
    var2 = 'quoted value again'
    var3 = this is not a "quoted" value

The default behavior of the library recognizes no escape sequences, but if the
`CONF_OPT_ESC` bit is set in the second argument to `conf_preset()` and
`conf_init()`, they are recognized in a quoted value; an unquoted value
supports no escape sequences. The supported sequences are:

    \'    \"    \?    \\    \0
    \a    \b    \f    \n    \r    \t    \v

with the same meanings as defined in C, and also include:

    \;    \#    \=

that are replaced with a semicolon, number sign and equal sign respectively.

Any leading and trailing whitespaces are omitted from an unquoted value; thus
only way to keep those spaces is to quote the value. Other whitespaces are kept
unchanged.


### 1.5. Boilerplate code

As already explained, using the library starts with invoking `conf_preset()` or
`conf_init()`. If you desire to provide a predefined set of sections and
variables with default values, call `conf_preset()` before calling
`conf_init()`. It is decided when calling `conf_preset()` or `conf_init()`
whether names are case-sensitive and escape sequences are recognized in quoted
values.

After reading a configuration file using `conf_init()`, a user program can
freely inspects variables using `conf_get()`, `conf_getbool()`,
`conf_getint()`, `conf_getuint()`, `conf_getreal()` and `conf_getstr()`.
`conf_get()` retrieves the value of a given variable and interprets it as
having the declared type of the variable. Other functions are useful when a
variable has the string type and a user code knows how to interpret it; when a
configuration description table is not used, all variables are assumed to have
the string type. If variables belonging to a specific section are frequently
referred to, `conf_section()` that changes the current section might help.

If a function returns an error indicator, an immediate call to `conf_errcode()`
returns the information about the error and `conf_errstr()` gives a string
describing an error code that is useful when constructing messages for users.

This library works on top of the memory library and if any function that
performs memory allocation fails to get necessary memory, an exception is
raised.

`conf.c` contains an example designed to use as many facilities of the library
as possible and a boilerplate code is given here:

    #define CONFFILE "test.conf"

    /* ... */

    conf_t tab[] = {
        "VarBool",          CONF_TYPE_BOOL, "yes",
        "VarInt",           CONF_TYPE_INT,  "255",
        "VarUint",          CONF_TYPE_UINT, "0xFFFF",
        "VarReal",          CONF_TYPE_REAL, "3.14159",
        "VarStr",           CONF_TYPE_STR,  "Global VarStr Default",
        "Section1.VarBool", CONF_TYPE_BOOL, "FALSE",
        "Section1.VarStr",  CONF_TYPE_STR,  "Section1.VarStr Default",
        "Section2.VarBool", CONF_TYPE_BOOL, "true",
        "Section2.VarReal", CONF_TYPE_REAL, "314159e-5",
        NULL,
    };
    size_t line;
    FILE *fp;

    if (conf_preset(tab, CONF_OPT_CASE | CONF_OPT_ESC) != CONF_ERR_OK) {
        fprintf(stderr, "test: %s\n", conf_errstr(conf_errcode()));
        conf_free();
        conf_hashreset();
        exit(EXIT_FAILURE);
    }

    fp = fopen(CONFFILE, "r");
    if (!fp) {
        fprintf(stderr, "test: failed to open %s for reading\n", CONFFILE);
        conf_free();
        conf_hashreset();
        exit(EXIT_FAILURE);
    }

    line = conf_init(fp, 0);
    fclose(fp);

    if (line != 0) {
        fprintf(stderr, "test:%s:%ld: %s\n", CONFFILE, (unsigned long)line,
                conf_errstr(conf_errcode()));
        conf_free();
        conf_hashreset();
        exit(EXIT_FAILURE);
    }

    /* ... sets an internal data structure properly
           according to what are read from configuration variables ... */

    conf_free();
    conf_hashreset();

    /* ... */

Even if this code defines the name of a configuration file as a macro, you may
hard-code it or make it determined from a program argument.

An array of the `conf_t` type, `tab` is a configuration description table. It
defines five variables in the global scope, each of which has the boolean,
integer, unsigned integer, real and string type, respectively. It defines two
sections named `Section1` and `Section2`, and four variables that belong to
them. The last value in each row is a default value for each variable. A null
pointer terminates defining the table.

`conf_preset()` delivers the table to the library. If a problem occurs,
`conf_preset()` returns an error code (other than `CONF_ERR_OK`), and you can
inspect it further using `conf_errcode()` and `conf_errstr()`. Do not forget
that this library is based on data structures using the memory library that
raises an exception if memory allocation fails.

`conf_init()` takes a stream (a `FILE` pointer), not a file name. This is
because taking a stream allows for its user to hand various kinds of files or
file-like objects, for example, a string connected to a stream which has no
file name.

Once `conf_init()` has done its job, the stream for the configuration file is
no longer necessary, so `fclose()` closes it.

`conf_init()` returns `0` if nothing is wrong, or the line number (that is
greater than 0) on which a problem occurs otherwise. You can use the return
value when issuing an error message.

Note that if the hash table supported by the hash library is used for other
purposes, it may not be desired to call `conf_hashreset()`. See `conf_free()`
and `conf_hashreset()` for more details. If you feel uncomfortable with several
instances of calls to `conf_free()` and `conf_hashreset()`, you can introduce a
label before clean-up code and jump to that label whenever cleaning-up is
required.


## 2. APIs

### 2.1. Types

#### `conf_t`

`conf_t` represents an element of a configuration description table; a
configuration table is the only way to specify types of variables as having
other than `CONF_TYPE_STR` (string type).

| Type     | Name   | Meaning                        |
|:--------:|:------:|:-------------------------------|
| `char *` | var    | section name and variable name |
| `int`    | type   | type of variable               |
| `char *` | defval | default value                  |

`var` specifies a section/variable name. The string has one of the following
two forms:

    variable
    section . variable

where whitespaces are allowed before and/or after a section and variable name.
The first form refers to a variable in the global section; there is no concept
of the _current_ section yet because `conf_section()` cannot be invoked before
`conf_preset()` or `conf_init()`. To mark the end of a table, set `var` to a
null pointer.

`type` specifies the type of a variable and should be one of `CONF_TYPE_BOOL`
(boolean value, `int`), `CONF_TYPE_INT` (signed integer, `long`),
`CONF_TYPE_UINT` (unsigned integer, `unsigned long`), `CONF_TYPE_REAL`
(floating-point number, `double`), and `CONF_TYPE_STR` (string, `char *`). Once
a variable is set to have a type, there is no way to change its type; thus, if
a variable is supposed to have various types depending on the context, set to
`CONF_TYPE_STR` and use `conf_conv()`. For `OPT_TYPE_INT` and `OPT_TYPE_UINT`,
the conversion of a value recognizes the C-style prefixes; numbers starting
with `0` are treated as octal, and those with `0x` or `0X` as hexadecimal.

`defval` specifies a default value for a variable that is used when a
configuration file dose not set that variable. It cannot be a null pointer but
an empty string is allowed. Note that `conf_preset()` that accepts a
configuration description table does not check if a default value has a proper
form for the type of a variable.

See the commented-out example in `conf.c` for more about a configuration
description table.


### 2.2. Constants

#### Types of values

These `enum` constants represent types for values:

| Name           | Meaning                                 |
|:--------------:|:----------------------------------------|
| CONF_TYPE_BOOL | boolean (`int`) type                    |
| CONF_TYPE_INT  | integer (`long`) type                   |
| CONF_TYPE_UINT | unsigned integer (`unsigned long`) type |
| CONF_TYPE_REAL | floating-point (`double`) type          |
| CONF_TYPE_STR  | string (`char *`) type                  |

See `conf_t` above.


#### Error codes

These `enum` constants represent error codes:

| Name            | Meaning                        |
|:---------------:|:-------------------------------|
| CONF_ERR_OK     | everything is okay             |
| CONF_ERR_FILE   | file not found                 |
| CONF_ERR_IO     | I/O error occurred             |
| CONF_ERR_SPACE  | space in section/variable name |
| CONF_ERR_CHAR   | invalid character encountered  |
| CONF_ERR_LINE   | invalid line encountered       |
| CONF_ERR_BSLASH | no following line for slicing  |
| CONF_ERR_SEC    | section not found              |
| CONF_ERR_VAR    | variable not found             |
| CONF_ERR_TYPE   | data type mismatch             |

See return values from API functions.


#### Masks for control options

These `enum` constants are used to mask control options:

| Name            | Meaning                                  |
|:---------------:|:-----------------------------------------|
| CONF_OPT_CASE   | case-sensitive variable/section name     |
| CONF_OPT_ESC    | supports escape sequence in quoted value |

See `conf_preset()` below.


### 2.3. Initializing configuration

#### `int conf_preset(const conf_t *tab, int ctrl)`

`conf_preset()` constructs a default set for configuration variables.

A user program can specify the default set of configuration variables
(including sections to which they belong and their types) with `conf_preset()`.
The table (an array, in fact) containing necessary information have the
`conf_t` type and called a _configuration description table_. For a detailed
explanation and examples, see `conf_t`. `conf_preset()`, if invoked, has to be
called before `conf_init()`. `conf_init()` does not have to be invoked if
`conf_preset()` is used.

`conf_preset()` remembers names that need to be recognized as sections and
variables, types of variables, and their default values. When `conf_init()`
processes a configuration file, a sections or variable that is not given via
`conf_preset()` is considered an error. Using `conf_preset()` is the only way
to let variables have other types than `CONF_TYPE_STR` (string type).

If `conf_preset()` not invoked, `conf_init()` accepts any section and variable
name (if they have a valid form) and all of variables are assumed to be of
`CONF_TYPE_STR` type.

`conf_preset()` also takes `ctrl` for controlling some behaviors of the
library, especially handling section/variable names and values. If the
`CONF_OPT_CASE` bit is set (that is, `CONF_OPT_CASE & ctrl` is not 0), section
and variable names are case-sensitive. If the `CONF_OPT_ESC` bit is set, some
forms of escape sequences are supported in a quoted value. The default behavior
is that section and variable names are case-insensitive and no escape sequences
are supported.

_`conf_preset()` does not warn that a default value for a variable does not
have an expected form for the variable's type. It is to be treated as an error
when retrieving the value via `conf_get()` or similar functions.

##### May raise

`assert_exceptfail` (see the assertion library from `cbl`) and `mem_exceptfail`
(see the memory library from `cbl`).

##### Takes

| Name | In/out | Meaning                         |
|:----:|:------:|:--------------------------------|
| tab  | in     | configuration description table |
| ctrl | in     | control code                    |

##### Returns

A success/failure indicator.

| Value         | Meaning |
|:-------------:|:--------|
| `CONF_ERR_OK` | success |
| others        | failure |


#### `size_t conf_init(FILE *fp, int ctrl)`

`conf_init()` reads a configuration file and constructs the configuration data
by analyzing the file.

For how `conf_init()` interacts with `conf_preset()`, see `conf_preset()`.

The default behavior of the library is that names are not case-insensitive and
escape sequences are not recognized. This behavior can be changed by setting
the `CONF_OPT_CASE` and `CONF_OPT_ESC` bits in `ctrl`, respectively; see also
`conf_preset()`.

If the control mode has been already set by `conf_preset()`, `conf_init()`
ignores `ctrl`.

##### May raise

`assert_exceptfail` (see the assertion library) and `mem_exceptfail` (see the
memory library).

##### Takes

| Name | In/out | Meaning                                            |
|:----:|:------:|:---------------------------------------------------|
| fp   | in     | file pointer from which configuration will be read |
| ctrl | in     | control code                                       |

##### Returns

A success/failure indicator.

| Value    | Meaning                                |
|:--------:|:---------------------------------------|
| `0`      | success                                |
| positive | line number on which an error occurred |


#### `void conf_free(void)`

`conf_free()` deallocates storages for the configuration data.

After `conf_free()` invoked, `conf_` functions should not be called without an
intervening call to `conf_preset()` or `conf_init()`.

_`conf_free()` does not reset the hash table used internally since it may be
used by other parts of the program. Invoking `hash_reset()` through
`conf_hashreset()` before program termination cleans up storages occupied by
the table._

##### May raise

`assert_exceptfail` (see the assertion library).

##### Takes

Nothing.

##### Returns

Nothing.


#### `void conf_hashreset(void)`

`conf_hashreset()` simply calls `hash_reset()` to reset the hash table.

As explained in `conf_free()`, `conf_free()` does not invoke `hash_reset()`
because the single hash table may be used by other parts of a user program.
Since requiring a reference to `hash_reset()` when using the configuration
library is inconsistent and inconvenient (e.g., a user code is obliged to
`#include` "`hash.h`"), `conf_hashreset()` is provided as a wrapper for
`hash_reset()`.

_Do not forget that the effect on the hash table caused by `conf_hashreset()`
is not limited to eliminating only what `conf_` functions adds to the table; it
completely cleans up the entire hash table._

##### May raise

Nothing.

##### Takes

Nothing.

##### Returns

Nothing.


### 2.3. Handling data from configuration

#### `const void *conf_conv(const char *val, int type)`

`conf_conv()` converts a string to an integer or floating-point number as
requested.

`type` should be `CONF_TYPE_BOOL` (which recognizes some forms of boolean
values), `CONF_TYPE_INT` (which indicates conversion to `signed long int`),
`CONT_TYPE_UINT` (conversion to `unsigned long int`), `CONF_TYPE_REAL`
(conversion to `double`) or `CONF_TYPE_STR` (no conversion necessary). The
expected forms for `CONF_TYPE_INT`, `CONF_TYPE_UINT` and `CONF_TYPE_REAL` are
respectively those for `strtol()`, `strtoul()` and `strtod()`. `CONF_TYPE_BOOL`
gives `1` for a string starting with `t`, `T`, `y`, `Y`, `1` and `0` for
others. `conf_conv()` returns a pointer to the storage that contains the
converted value (an integer, floating-point number or string) and its caller
is obliged to convert the pointer properly (to `const int *`, `const long *`,
`const unsigned long *`, `const double *` and `const char *`) before use. If
the conversion fails, `conf_conv()` returns a null pointer and sets
`CONF_ERR_TYPE` as an error code.

_A subsequent call to `conf_getbool()`, `conf_getint()`, `conf_getuint()` and
`conf_getreal()` may overwrite the contents of the buffer pointed to by the
resulting pointer. Similarly, a subsequent call to `conf_conv()` and
`conf_get()` may overwrite the contents of the buffer pointed to by the
resulting pointer unless the type is `CONF_TYPE_STR`._

##### May raise

Nothing.

##### Takes

| Name | In/out | Meaning                                          |
|:----:|:------:|:-------------------------------------------------|
| val  | in     | string to convert                                |
| type | in     | type based on which conversion will be performed |

##### Returns

A pointer to storage that contains the result, or null pointer.


#### `const void *conf_get(const char *var)`

`conf_get()` retrieves a value from a section/variable name.

In a program (e.g., when using `conf_get()`), variables can be referred to
using one of the following forms:

    variable
    . variable
    section . variable

where whitespaces are optional before and after section and variable names. The
first form refers to a variable belonging to the _current_ section; the current
section can be set by `conf_section()`. The second form refers to a variable
belonging to the global section. The last form refers to a variable belonging
to a specific section.

_A subsequent call to `conf_conv()` and `conf_get()` may overwrite the contents
of the buffer pointed to by the resulting pointer unless the type is
`CONF_TYPE_STR`. Similarly, a subsequent call to `conf_getbool()`,
`conf_getint()`, `conf_getuint()` and `conf_getreal()` may overwrite the
contents of the buffer pointed to by the resulting pointer._

##### May raise

`assert_exceptfail` (see the assertion library) and `mem_exceptfail` (see the
memory library).

##### Takes

| Name | In/out | Meaning               |
|:----:|:------:|:----------------------|
| var  | in     | section/variable name |

##### Returns

A pointer to storage that contains a value, or null pointer


#### `int conf_getbool(const char *var, int errval)`

`conf_getbool()` retrieves a boolean value from a section/variable name.

Every value for a variable is stored in a string form, and `conf_getbool()`
converts it to a boolean value; the result is `1` (indicating true) if the
string starts with `t`, `T`, `y`, `Y` or `1` ignoring any leading spaces and
`0` (indicating false) otherwise. If there is no variable with the given name
or the preset type of the variable is not `CONF_TYPE_BOOL`, the value of
`errval` is returned.

For how to refer to variables in a program, see `conf_get()`.

##### May raise

`assert_exceptfail` (see the assertion library) and `mem_exceptfail` (see the
memory library).

##### Takes

| Name   | In/out | Meaning                    |
|:------:|:------:|:---------------------------|
| var    | in     | section/variable name      |
| errval | in     | value returned as an error |

##### Returns

A converted result or `errval`.


#### `long conf_getint(const char *var, long errval)`

`conf_getint()` retrieves an integral value from a section/variable name.

Every value for a variable is stored in a string form, and `conf_getint()`
converts it to an integer using `strtol()` from `<stdlib.h>`. If there is no
variable with the given name or the preset type of the variable is not
`CONF_TYPE_INT`, the value of `errval` is returned.

For how to refer to variables in a program, see `conf_get()`.

##### May raise

`assert_exceptfail` (see the assertion library) and `mem_exceptfail` (see the
memory library).

##### Takes

| Name   | In/out | Meaning                    |
|:------:|:------:|:---------------------------|
| var    | in     | section/variable name      |
| errval | in     | value returned as an error |

##### Returns

A converted result or `errval`.


#### `unsigned long conf_getuint(const char *var, unsigned long errval)`

`conf_getuint()` retrieves an unsigned integral value from a section/variable
name.

Every value for a variable is stored in a string form, and `conf_getuint()`
converts it to an unsigned integer using `strtoul()` from `<stdlib.h>`. If
there is no variable with the given name or the preset type of the variable is
not `CONF_TYPE_UINT`, the value of `errval` is returned.

For how to refer to variables in a program, see `conf_get()`.

##### May raise

`assert_exceptfail` (see the assertion library) and `mem_exceptfail` (see the
memory library).

##### Takes

| Name   | In/out | Meaning                    |
|:------:|:------:|:---------------------------|
| var    | in     | section/variable name      |
| errval | in     | value returned as an error |

##### Returns

A converted result or `errval`.


#### `double conf_getreal(const char *var, double errval)`

`conf_getreal()` retrieves a real value from a section/variable name.

Every value for a variable is stored in a string form, and `conf_getreal()`
converts it to a floating-point number using `strtod()` from `<stdlib.h>`. If
there is no variable with the given name or the preset type of the variable is
not `CONF_TYPE_REAL`, the value of `errval` is returned; `HUGE_VAL` defined in
`<math.h>` would be a nice choice for `errval`.

For how to refer to variables in a program, see `conf_get()`.

##### May raise

`assert_exceptfail` (see the assertion library) and `mem_exceptfail` (see the
memory library).

##### Takes

| Name   | In/out | Meaning                    |
|:------:|:------:|:---------------------------|
| var    | in     | section/variable name      |
| errval | in     | value returned as an error |

##### Returns

A converted result or `errval`.


#### `const char *conf_getstr(const char *var)`

`conf_getstr()` retrieves a string from a section/variable name.

Every value for a variable is stored in a string form, thus `conf_getstr()`
performs no conversion. If there is no variable with the given name or the
preset type of the variable is not `CONF_TYPE_STR`, a null pointer is returned.

For how to refer to variables in a program, see `conf_get()`.

##### May raise

`assert_exceptfail` (see the assertion library) and `mem_exceptfail` (see the
memory library).

##### Takes

| Name   | In/out | Meaning                    |
|:------:|:------:|:---------------------------|
| var    | in     | section/variable name      |

##### Returns

A string or null pointer.


#### `int conf_set(const char *secvar, const char *value)`

`conf_set()` inserts or replaces a value associated with a variable.

If `conf_preset()` has been invoked, `conf_set()` is able to only replace a
value associated with an existing variable, which means an error code is
returned when a user tries to insert a new variable and its value (possibly
with a new section). `conf_set()` is allowed to insert a new variable-value
pair otherwise.

For how to refer to variables in a program, see `conf_get()`.

_When `conf_preset()` invoked, `conf_set()` does not check if a given value is
appropriate to the preset type of a variable. That mismatch is to be detected
when `conf_get()` or similar functions called later for the variable._

##### May raise

`assert_exceptfail` (see the assertion library) and `mem_exceptfail` (see the
memory library).

##### Takes

| Name   | In/out | Meaning                    |
|:------:|:------:|:---------------------------|
| secvar | in     | section/variable name      |
| value  | in     | value to store             |

##### Returns

A success/failure indicator.

| Value         | Meaning |
|:-------------:|:--------|
| `CONF_ERR_OK` | success |
| others        | failure |


#### `int conf_section(const char *sec)`

`conf_section()` sets the current section to a given section.

The global section can be set as the current section by giving an empty string
`""`. `conf_section()` affects how `conf_get()`, `conf_getbool()`,
`conf_getint()`, `confgetuint()`, `confgetreal()`, `confgetstr()` and
`conf_set()` work.

##### May raise

Nothing.

##### Takes

| Name | In/out | Meaning                                    |
|:----:|:------:|:-------------------------------------------|
| sec  | in     | section name to set as the current section |

##### Returns

A success/failure indicator.

| Value         | Meaning |
|:-------------:|:--------|
| `CONF_ERR_OK` | success |
| others        | failure |


### 2.4. Handling errors

#### `int conf_errcode(void)`

`conf_errcode()` returns an error code.

Every function in this library sets the internal error variable as it performs
its operation. Unlike errno provided by `<errno.h>`, the error variable of this
library is set to `CONF_ERR_OK` before starting an operation, thus a user code
need not to clear it before calling `conf_` functions.

When using a function returning an error code (of `int` type), the returned
value is the same as what `conf_errcode()` will return if there is no
intervening call to a `conf_` function between them. When using a function
returning a pointer, the only way to get what the error has been occurred is to
use `conf_errcode()`.

The following code fragment shows an example for how to use `conf_errcode()`
and `conf_errstr()`:

    fp = fopen(conf, "r");
    if (!fp)
        fprintf(stderr, "%s:%s: %s\n", prg, conf, conf_errstr(CONF_ERR_FILE));
    line = conf_init(fp, CONF_OPT_CASE | CONF_OPT_ESC);
    if (line != 0)
        fprintf(stderr, "%s:%s:%lu: %s\n", prg, conf, line, conf_errstr(conf_errcode()));

##### May raise

Nothing.

##### Takes

Nothing.

##### Returns

The current error code.


#### `const char *conf_errstr(int code)`

`conf_errstr()` returns an error message for a given error code.

##### May raise

`assert_exceptfail` (see the assertion library).

##### Takes

| Name | In/out | Meaning                                         |
|:----:|:------:|:------------------------------------------------|
| code | in     | error code for which a message will be returned |

##### Returns

An error message.


## 3. Future directions

### 3.1. Recoverable errors

The current implementation does not provide a way to recover from errors like
encountering unrecognized sections or variables. Recovering from them is
sometimes necessary; for example, a programmer might want to issue a diagnostic
message when a user uses an old version of the configuration file format, or to
construct a certain part of the configuration file format dynamically depending
on other parts of it.


### 3.2. Minor changes

`table_new()` used by the configuration library to create tables for
configuration data takes a hint for the expected size of the table to create.
Even if the performance is not a big issue in this library, providing a
reasonable one to `table_new()` is necessary.


## 4. Contact me

Visit [`code.woong.org`](http://code.woong.org) to get the latest version of
this library. Any comments about the library are welcomed. If you have a
proposal or question on the library just email me, and I will reply as soon as
possible.


## 5. Copyright

For the copyright issues, see `LICENSE.md`.
