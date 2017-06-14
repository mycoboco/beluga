C data structure library: dwa
=============================

This document specifies the double-word arithmetic library which belongs to C
data structure library. Algorithms used for some operations are hired from
[C Interfaces and Implementations](https://sites.google.com/site/cinterfacesimplementations/)
by David Hanson and [Hacker's Delight](http://www.hackersdelight.org) by
Henry S. Warren.

The internals of the library is not to be explained here. Explanations for
APIs, however, are given to aid the use of the library.


## 1. Introduction

This library implements double-word arithmetic that provides all of the basic
operations on signed and unsigned integers whose precision can be large as
twice as the biggest native integer type; for example, with 64-bit `long long`,
the library lets you have 128-bit integer types. It is intended to mimic native
integer arithmetic on common systems and has the following properties:

- Signed operations work on the
  [two'2 complement representation](https://en.wikipedia.org/wiki/Two%27s_complement),
  which has positive and negative extreme values asymmetric;
- No flag or trap to detect overflow is provided; overflows are ignored
  silently truncating excess bits. If the detection is necessary, a proper
  check has to be placed _before_ operations; and
- The double-word integer type named `dwa_t` is a `struct` type. That means its
  value is copied, as a native integer value, when passing through arguments or
  from functions. It might sound inefficient, but assuming that the underlying
  single-word integer types, `dwa_ubase_t` and `dwa_base_t` (called
  _base types_ in this document) are native ones (as opposed to being
  emulated), block-copying of such size is fast enough on most machines.

This library reserves identifiers starting with `dwa_` and `DWA_`, and imports
the assertion library (which requires the exception library). Some behaviors of
the library are controlled by macros `DWA_USE_W` and `DWA_BASE_T`; see below
for details.


### 1.1. How to use the library

Choosing a `struct` instead of an array for the representation for double-word
integers simplifies the use of the library because no storage management is
required.

All functions with `u` in their names, `dwa_addu` and `dwa_fromuint` for
example, always perform unsigned arithmetic, while their counterparts without
`u` interpret internal bit patterns as 2's complement forms and perform signed
operations.

You can construct a new double-word integer from a native integer value
using `dwa_fromuint()` or `dwa_fromint()`, from a string representation using
`dwa_fromstr()`, or from a floating-point value using `dwa_fromfp()`.

All operations, except unary `+` and `!`, that C offers with usual operators
are supported:

- unary `-`: `dwa_neg()`;
- `+`: `dwa_addu()` and `dwa_add()`;
- binary `-`: `dwa_subu()` and `dwa_sub()`;
- `*`: `dwa_mulu()` and `dwa_mul()`;
- `/`: `dwa_divu()` and `dwa_div()`;
- `~`: `dwa_bcom()`;
- `<<`: `dwa_lsh()`;
- `>>`: `dwa_rshl()` (for logical shift) or `dwa_rsha()` (for
  arithmetic shift);
- `&`, `^` and `|`: `dwa_bit()`; and
- `<`, `<=`, `==`, `!=`, `>=`, `>`: `dwa_cmpu()` and `dwa_cmp()`.

The logical negation operator(`!`) can be achieved by a comparison to zero
using `dwa_cmpu()`.

`dwa_touint()` or `dwa_toint()` extracts the least significant word from a
`dwa_t` value. Conversion to a string representation is supported by
`dwa_tostru()` or `dwa_tostr()`, and to a `long double` value by `dwa_tofpu()`
or `dwa_tofp()`.

Thanks to two's complement representation, conversions between
signed and unsigned values are unnecessary. For example, this code

    long long foo = -1;
    unsigned long long bar = (unsigned long long)foo + 1;

can be emulated by

    dwa_t foo = dwa_fromint(-1);
    dwa_t bar = dwa_sumu(foo, 1);

on an environment that lacks `long long`.


### 1.2. Boilerplate code

The following example code shows how to perform 64-bit operations by macroizing
`long long` operations, which resorts to using `dwa_t` when necessary:

    #include <limits.h>
    #include <stdio.h>
    #include <cdsl/dwa.h>

    #if LLONG_MAX
        typedef long long sint_t;
        typedef unsigned long long uint_t;

        #define umax ULLONG_MAX
        #define max  LLONG_MAX
        #define min  LLONG_MIN

        #define is(x)    (x)
        #define au(x, y) ((x) + (y))
        #define eu(x, y) ((x) == (y))
        #define es(x, y) ((x) == (y))
        #define str(x)   (sprintf(buf, "%lld", (x)))
    #else
        typedef dwa_t sint_t;
        typedef dwa_t uint_t;

        #define umax dwa_umax
        #define max  dwa_max
        #define min  dwa_min

        #define is(x)    (dwa_fromint(x))
        #define au(x, y) (dwa_addu((x), (y)))
        #define eu(x, y) (dwa_cmpu((x), (y)) == 0)
        #define es(x, y) (dwa_cmp((x), (y)) == 0)
        #define str(x)   (dwa_tostr(buf, (x), 10))
    #endif

    char buf[DWA_BUFSIZE];
    sint_t foo = is(-1);             // long long foo = -1;
    uint_t bar = max;                // unsigned long long bar = ULLONG_MAX;

    dwa_prep();

    printf("%d\n", eu(foo, bar));    // printf("%d\n", foo == bar);
    printf("%d\n", es(foo, bar));    // printf("%d\n", foo == (long long)bar);

    str(au(foo, bar));
    puts(buf);                       // printf("%lld\n", foo + bar);

`dwa_prep()` prepares `dwa_umax`, `dwa_max` and `dwa_min` properly.

`DWA_BUFSIZE` is useful when preparing a buffer to contain string
representations of double-word integers. Note how conversion and
[type-punning](https://en.wikipedia.org/wiki/Type_punning) between signed and
unsigned types are achieved by properly selected `dwa` functions.

See [`beluga`](https://github.com/mycoboco/beluga) for a more complete example.


## 2. APIs

### 2.1. Types

#### `dwa_t`

`dwa_t` is a `struct` that represents a double-word integer value.

#### `dwa_ubase_t` and `dwa_base_t`

The base types, `dwa_ubase_t` and `dwa_base_t` are initially defined as
synonyms for `unsigned long` and `signed long` respectively because the
intended purpose of `dwa` is to implement 64-bit arithmetic on a pure 32-bit
system where a compiler supports no `long long`. You can, however, change those
types for your needs as long as the chosen types have a conversion rank equal
to or greater than `int`, which includes `int`, `long` and `long long`.

The base types are defined in the code as follows:

    typedef unsigned DWA_BASE_T dwa_ubase_t;
    typedef signed   DWA_BASE_T dwa_base_t;

and defining the macro `DWA_BASE_T` (through `-D` compiler option, for example)
when compiling the library controls them; defining `DWA_BASE_T` as `long long`
sets `dwa_ubase_t` and `dwa_base_t` to be `unsigned long long` and
`signed long` respectively.


### 2.2. Width and useful values

#### `DWA_WIDTH`

The number of bits `dwa_t` contains to represent double-word integers can be
accessed via a macro named `DWA_WIDTH`. For example, you can get a `dwa_t`
value with `n` lower bits set to 1 as follows:

    dwa_rshl(dwa_fromuint(-1), DWA_WIDTH - n)

or

    dwa_rshl(dwa_neg1, DWA_WIDTH - n)

; see below for `dwa_neg1`.

#### `dwa_umax`, `dwa_max` and `dwa_min`

The range of _unsigned_ `dwa_t` values is [0, `dwa_umax`] and that of _signed_
one [`dwa_min`, `dwa_max`]. These globals, even if not constants, are intended
to play roles of macros from `<limits.h>`.

`dwa_prep()` has to be invoked for proper initialization before use, and trying
to modify the variables results in undefined behavior.

#### `dwa_0`, `dwa_1` and `dwa_neg1`

When mixing double-word integers with native ones in expressions, converting
integer constants 0, 1 and -1 to double-word ones is frequent and may lead to
performance degradation. These variables can be used to replace, respectively,
calls to `dwa_fromint(0)`, `dwa_fromint(1)` and `dwa_fromint(-1)`, where
`dwa_fromint()` constructs double-word integers from signed base type values.

`dwa_prep()` has to be invoked for proper initialization before use, and trying
to modify the variables results in undefined behavior.

#### `void dwa_prep(void)`

`dwa_prep()` prepares `dwa_umax`, `dwa_max` and `dwa_min` for limit values, and
`dwa_0`, `dwa_1` and `dwa_neg1` for constant values, and must precede their
use.

`dwa_prep()` need not be invoked if those globals are not necessary, and
redundant calls to it have no harm; it is idempotent.

### 2.3. Conversion from and to native integers

#### `dwa_t dwa_fromuint(dwa_ubase_t v)`

`dwa_fromuint()` constructs a new double-word value whose least significant
word comes from unsigned `v`.

##### May raise

Nothing.

##### Takes

| Name | In/out | Meaning                                             |
|:----:|:------:|:----------------------------------------------------|
| v    | in     | single-word value for the least significant word    |

##### Returns

The constructed double-word value.


#### `dwa_t dwa_fromint(dwa_base_t v)`

`dwa_fromint()` constructs a new double-word value whose least significant
word comes from signed `v`.

##### May raise

Nothing.

##### Takes

| Name | In/out | Meaning                                             |
|:----:|:------:|:----------------------------------------------------|
| v    | in     | single-word value for the least significant word    |

##### Returns

The constructed double-word value.


#### `dwa_ubase_t dwa_touint(dwa_t x)`

`dwa_touint()` extracts the least significant word from unsigned `x`.

##### May raise

Nothing.

##### Takes

| Name | In/out | Meaning                                                    |
|:----:|:------:|:-----------------------------------------------------------|
| x    | in     | double-word value from which a single-word value extracted |

##### Returns

The extracted unsigned single-word value.


#### `dwa_base_t dwa_toint(dwa_t x)`

`dwa_toint()` extracts the value of the least significant word from signed `x`.

##### May raise

Nothing.

##### Takes

| Name | In/out | Meaning                                                    |
|:----:|:------:|:-----------------------------------------------------------|
| x    | in     | double-word value from which a single-word value extracted |

##### Returns

The extracted signed single-word value.


### 2.4. Arithmetic operations

#### `dwa_t dwa_neg(dwa_t x)`

`dwa_neg()` negates `x` by constructing its 2's complement form.

##### May raise

Nothing.

##### Takes

| Name | In/out | Meaning                     |
|:----:|:------:|:----------------------------|
| x    | in     | double-word value to negate |

##### Returns

The negated double-word value.


#### `dwa_t dwa_addu(dwa_t x, dwa_t y)`

`dwa_addu()` computes `x` + `y`.

##### May raise

Nothing.

##### Takes

| Name | In/out | Meaning                                   |
|:----:|:------:|:------------------------------------------|
| x    | in     | unsigned double-word operand for addition |
| y    | in     | unsigned double-word operand for addition |

##### Returns

The unsigned result of `x` + `y`.


#### `dwa_t dwa_add(dwa_t x, dwa_t y)`

`dwa_add()` computes `x` + `y`.

##### May raise

Nothing.

##### Takes

| Name | In/out | Meaning                                 |
|:----:|:------:|:----------------------------------------|
| x    | in     | signed double-word operand for addition |
| y    | in     | signed double-word operand for addition |

##### Returns

The signed result of `x` + `y`.


#### `dwa_t dwa_subu(dwa_t x, dwa_t y)`

`dwa_subu()` computes `x` - `y`.

##### May raise

Nothing.

##### Takes

| Name | In/out | Meaning                                      |
|:----:|:------:|:---------------------------------------------|
| x    | in     | unsigned double-word operand for subtraction |
| y    | in     | unsigned double-word operand for subtraction |

##### Returns

The unsigned result of `x` - `y`.


#### `dwa_t dwa_sub(dwa_t x, dwa_t y)`

`dwa_sub()` computes `x` - `y`.

##### May raise

Nothing.

##### Takes

| Name | In/out | Meaning                                    |
|:----:|:------:|:-------------------------------------------|
| x    | in     | signed double-word operand for subtraction |
| y    | in     | signed double-word operand for subtraction |

##### Returns

The signed result of `x` - `y`.


#### `dwa_t dwa_mulu(dwa_t x, dwa_t y)`

`dwa_mulu()` computes `x` * `y`.

##### May raise

Nothing.

##### Takes

| Name | In/out | Meaning                                         |
|:----:|:------:|:------------------------------------------------|
| x    | in     | unsigned double-word operand for multiplication |
| y    | in     | unsigned double-word operand for multiplication |

##### Returns

The unsigned result of `x` * `y`.


#### `dwa_t dwa_mul(dwa_t x, dwa_t y)`

`dwa_mul()` computes `x` * `y`.

##### May raise

Nothing.

##### Takes

| Name | In/out | Meaning                                       |
|:----:|:------:|:----------------------------------------------|
| x    | in     | signed double-word operand for multiplication |
| y    | in     | signed double-word operand for multiplication |

##### Returns

The signed result of `x` * `y`.


#### `dwa_t dwa_divu(dwa_t x, dwa_t y, int mod)`

`dwa_divu()` computes `x` / `y` or `x` % `y`. Setting `mod` to 0 lets
`dwa_divu()` compute a quotient, and a remainder otherwise. The fractional part
of a quotient is truncated.

##### May raise

Nothing.

##### Takes

| Name | In/out | Meaning                                      |
|:----:|:------:|:---------------------------------------------|
| x    | in     | unsigned double-word operand for division    |
| y    | in     | unsigned double-word operand for division    |
| mod  | in     | 0 computes a quotient; a remainder otherwise |

##### Returns

The unsigned result of `x` / `y` or `x` % `y`.


#### `dwa_t dwa_div(dwa_t x, dwa_t y, int mod)`

`dwa_div()` computes `x` / `y` or `x` % `y`. Setting `mod` to 0 lets
`dwa_div()` compute a quotient, and a remainder otherwise. The fractional part
of a quotient is truncated.

##### May raise

Nothing.

##### Takes

| Name | In/out | Meaning                                      |
|:----:|:------:|:---------------------------------------------|
| x    | in     | signed double-word operand for division      |
| y    | in     | signed double-word operand for division      |
| mod  | in     | 0 computes a quotient; a remainder otherwise |

##### Returns

The signed result of `x` / `y` or `x` % `y`.


### 2.5. Bitwise operations

#### `dwa_t dwa_bcom(dwa_t x)`

`dwa_bcom()` negates each bits of `x` to compute its
[1s' complement](https://en.wikipedia.org/wiki/Ones%27_complement).

##### May raise

Nothing.

##### Takes

| Name | In/out | Meaning                     |
|:----:|:------:|:----------------------------|
| x    | in     | double-word value to negate |

##### Returns

The 1s' complement of `x`.


#### `dwa_t dwa_lsh(dwa_t x, int n)`

`dwa_lsh()` computes `x` << `n`, which is `x` left-shited `n` bit positions.
Vacated bits are filled with zeros. `n` must be equal to or greater than 0, and
be less than the number of bits in `dwa_t`. The behavior is undefined
otherwise.

##### May raise

Nothing.

##### Takes

| Name | In/out | Meaning                         |
|:----:|:------:|:--------------------------------|
| x    | in     | double-word value to left-shift |
| n    | in     | bit positions for shift         |

##### Returns

The left-shifted result of `x`.


#### `dwa_t dwa_rshl(dwa_t x, int n)`

`dwa_rshl()` computes `x` >> `n`, which is `x` right-shited `n` bit positions.
Vacated bits are filled with zeros, thus a logical shift is performed. `n` must
be equal to or greater than 0, and be less than the number of bits in `dwa_t`.
The behavior is undefined otherwise.

##### May raise

Nothing.

##### Takes

| Name | In/out | Meaning                          |
|:----:|:------:|:---------------------------------|
| x    | in     | double-word value to right-shift |
| n    | in     | bit positions for shift          |

##### Returns

The right-shifted result of `x`.


#### `dwa_t dwa_rsha(dwa_t x, int n)`

`dwa_rsha()` computes `x` >> `n`, which is `x` right-shited `n` bit positions.
Vacated bits are filled with copies of the most significant bit, which is
called the [sign extension](https://en.wikipedia.org/wiki/Sign_extension), and
thus an arithmetic shift is performed. `n` must be equal to or greater than 0,
and be less than the number of bits in `dwa_t`. The behavior is undefined
otherwise.

##### May raise

Nothing.

##### Takes

| Name | In/out | Meaning                          |
|:----:|:------:|:---------------------------------|
| x    | in     | double-word value to right-shift |
| n    | in     | bit positions for shift          |

##### Returns

The right-shifted result of `x`.


#### `dwa_t dwa_bit(dwa_t x, dwa_t y, int op)`

`dwa_bit()` computes `x` & `n`, `x` ^ `y` or `x` | `y` depending on the value
of `op`: The macro `DWA_AND` denotes the bitwise AND, `DWA_XOR` the bitwise
XOR and `DWA_OR` the bitwise OR.

##### May raise

`assert_exceptfail` (see the assertion library).

##### Takes

| Name | In/out | Meaning                                   |
|:----:|:------:|:------------------------------------------|
| x    | in     | double-word operand for bitwise operation |
| y    | in     | double-word operand for bitwise operation |
| op   | in     | type of bitwise operation                 |

##### Returns

The result of a bitwise operation.


### 2.6. Comparisons

#### `dwa_t dwa_cmpu(dwa_t x, dwa_t y)`

`dwa_cmpu()` performs an unsigned comparison of `x` and `y`.

##### May raise

Nothing.

##### Takes

| Name | In/out | Meaning                                     |
|:----:|:------:|:--------------------------------------------|
| x    | in     | unsigned double-word operand for comparison |
| y    | in     | unsigned double-word operand for comparison |

##### Returns

The comparison result.

| Value    | Meaning                 |
|:--------:|:-----------------------:|
| 0        | `x` equals to `y`       |
| negative | `x` is less than `y`    |
| positive | `x` is greater than `y` |


#### `dwa_t dwa_cmp(dwa_t x, dwa_t y)`

`dwa_cmp()` performs a signed comparison of `x` and `y`.

##### May raise

Nothing.

##### Takes

| Name | In/out | Meaning                                   |
|:----:|:------:|:------------------------------------------|
| x    | in     | signed double-word operand for comparison |
| y    | in     | signed double-word operand for comparison |

##### Returns

The comparison result.

| Value    | Meaning                 |
|:--------:|:-----------------------:|
| 0        | `x` equals to `y`       |
| negative | `x` is less than `y`    |
| positive | `x` is greater than `y` |


### 2.7. Conversion from and to strings

#### `char *dwa_tostru(char *s, dwa_t x, int radix)`

`dwa_tostru()` converts an unsigned double-word integer to a string
representation.

`s`, if not `NULL`, must have a space enough to contain the resulting string
representation including a terminating `NUL`. No `+` sign is placed in the
result. The macro `DWA_BUFSIZE` is useful for allocating the conversion buffer;
it specifies the size enough to convert double-word integers in any radix. That
is,

    char s[DWA_BUFSIZE];
    dwa_tostru(s, x, radix);

is guaranteed to be safe for any valid `radix` values.

If `s` is `NULL`, `dwa_tostru()` uses an internal buffer that is statically
allocated, which makes `dwa_tostru()` non-reentrant. A subsequent call to
`dwa_tostru()` or `dwa_tostr()` with `s` set to `NULL` might overwrite the
internal buffer.

`radix` must be between 2 and 36 inclusive. When it is greater than 10,
lowercase characters from `a` to `z` are used to represent digits from 10 to
35.

##### May raise

`assert_exceptfail` (see the assertion library).

##### Takes

| Name  | In/out | Meaning                                |
|:-----:|:------:|:---------------------------------------|
| s     | out    | buffer to contain the resulting string |
| x     | in     | unsigned double-word value to convert  |
| radix | in     | radix for string representation        |

##### Returns

The converted string.


#### `char *dwa_tostr(char *s, dwa_t x, int radix)`

`dwa_tostr()` converts a signed double-word integer to a string representation.

`s`, if not `NULL`, must have a space enough to contain the resulting string
representation including a terminating `NUL`. A `-` sign is placed for negative
results, but a `+` sign is never placed. The macro `DWA_BUFSIZE` is useful for
allocating the conversion buffer; it specifies the size enough to convert
double-word integers in any radix. That is,

    char s[DWA_BUFSIZE];
    dwa_tostr(s, x, radix);

is guaranteed to be safe for any valid `radix` values.

If `s` is `NULL`, `dwa_tostr()` uses an internal buffer that is statically
allocated, which makes `dwa_tostr()` non-reentrant. A subsequent call to
`dwa_tostr()` or `dwa_tostru()` with `s` set to `NULL` might overwrite the
internal buffer.

`radix` must be between 2 and 36 inclusive. When it is greater than 10,
lowercase characters from `a` to `z` are used to represent digits from 10 to
35.

##### May raise

`assert_exceptfail` (see the assertion library).

##### Takes

| Name  | In/out | Meaning                                |
|:-----:|:------:|:---------------------------------------|
| s     | out    | buffer to contain the resulting string |
| x     | in     | signed double-word value to convert    |
| radix | in     | radix for string representation        |

##### Returns

The converted string.


#### `dwa_t dwa_fromstr(const char *str, char **end, int radix)`

`dwa_fromstr()` constructs a new double-word integer from the initial part of a
string representation in a similar way to
[`strtol()`](http://man7.org/linux/man-pages/man3/strtol.3.html) except that
`dwa_fromstr()` never touches `errno`. The following specification is almost
copied from `strtol()`'s `man` page.

`radix` must be between 2 and 36 inclusive, or be the special value 0.

The string in `str` may begin with an arbitrary amount of white space (as
determined by `isspace()`) followed by a single optional `+` or `-` sign. If
`radix` is zero or 16, the string may then include a `0x` prefix, and the
number will be read in radix 16; otherwise, a zero radix is taken as 10 unless
the next character is `0`, which case it is taken as 8.

The remainder of the string is converted to a double-word value in the obvious
manner, stopping at the first non-valid character or at the first digit to
incur overflow. In radices above 10, the characters from `A` to `Z` in either
uppercase or lowercase represent digits from 10 to 35.

If `end` is not `NULL`, `dwa_fromstr()` stores in `end` the address of the
first character to stop the conversion. If there were no digits at all,
`dwa_fromstr()` stores the original value of `str` in `*end` and returns 0. In
particular, if `*str` is not `'\0'` but `**end` is `'\0'` on return, the entire
string is valid.

##### May raise

`assert_exceptfail` (see the assertion library).

##### Takes

| Name  | In/out | Meaning                                               |
|:-----:|:------:|:------------------------------------------------------|
| str   | in     | string to convert                                     |
| end   | out    | object into which pointer to invalid character stored |
| radix | in     | radix for string representation                       |

##### Returns

The converted double-word value.


### 2.8. Conversion from and to floating-points

To avoid dependency on a specific format of floating-point types, conversions
between double-word integers and floating-point values are performed
arithmetically and work best with radix-2 floating-point formats. If a value
from the conversion cannot be represented, the behavior is undefined. Note that
this includes special values like
[NaNs](https://en.wikipedia.org/wiki/NaN) and infinities.

#### `dwa_t dwa_fromfp(long double)`

`dwa_fromfp()` constructs a new double-word integer from a floating-point
value.

##### May raise

Nothing.

##### Takes

| Name | In/out | Meaning                         |
|:----:|:------:|:--------------------------------|
| v    | in     | floating-point value to convert |

##### Returns

The converted double-word value.


#### `long double dwa_tofpu(dwa_t x)`

`dwa_tofpu()` converts an unsigned double-word value to a floating-point value.

##### May raise

Nothing.

##### Takes

| Name | In/out | Meaning                               |
|:----:|:------:|:--------------------------------------|
| x    | in     | unsigned double-word value to convert |

##### Returns

The converted floating-point value.


#### `long doule dwa_tofp(dwa_t x)`

`dwa_tofpu()` converts a signed double-word value to a floating-point value.

##### May raise

Nothing.

##### Takes

| Name | In/out | Meaning                             |
|:----:|:------:|:------------------------------------|
| x    | in     | signed double-word value to convert |

##### Returns

The converted floating-point value.


## 3. The `DWA_USE_W` macro

Internally, the `dwa` library stores a double-word integer in radix-256 digits
and puts the least significant digit first (_little-endian_). If the underlying
machine has 8-bit bytes and uses the little-endian byte order, however, it is
much more efficient for some operations to consider a single word a single
digit. Defining `DWA_USE_W` when compiling the library allows this; see
`INSTALL.md` for details.


## 4. Contact me

Visit [`code.woong.org`](http://code.woong.org) to get the latest version of
this library. Any comments about the library are welcomed. If you have a
proposal or question on the library just email me, and I will reply as soon as
possible.


## 5. Copyright

For the copyright issues, see `LICENSE.md`.
