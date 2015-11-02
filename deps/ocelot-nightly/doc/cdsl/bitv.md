C data structure library: bit-vector
====================================

This document specifies the bit-vector library which belongs to C data
structure library. The basic structure is from David Hanson's book,
[C Interfaces and Implementations](https://sites.google.com/site/cinterfacesimplementations/).
I modified the original implementation to conform to the C standard and to
enhance its readability.

The book explains its design and implementation in a very comprehensive way.
Not to mention the copyright issues, the internals of the library is not to be
explained here. Explanations for APIs, however, are given to aid the use of the
library.


## 1. Introduction

This library implements a [bit-vector](http://en.wikipedia.org/wiki/Bit_array)
that is a set of integers. An unsigned integer type like `unsigned long` or a
bit-field in a `struct` or `union` is often used to represent such a set, and
various [bitwise operators](http://en.wikipedia.org/wiki/Bitwise_operation)
serve set operations; for example, the bitwise OR operator effectively provides
a union operation. This approach, however, imposes a restriction that the size
of a set should be limited by that of the primitive integer type chosen. This
library gets rid of such a limit and allows users to use a set of integers with
an arbitrary size at the cost of dynamic memory allocation and a more complex
data structure than a simple integer type.

This library reserves identifiers starting with `bitv_` and `BITV_`, and
imports the assertion library (which requires the exception library) and the
memory library.


### 1.1. How to use the library

Similarly to other data structure libraries, a typical use of the bit-vector
library follows this sequence: create, use and destroy.

In general, a null pointer given to an argument expecting a bit-vector is
considered an error which results in an assertion failure, but the functions
for set operations (`bitv_union()`, `bitv_inter()`, `bitv_minus()` and
`bitv_diff()`) take a null pointer as a valid argument and treat it as
representing an empty (all-bits-cleared) bit-vector. Also note that they always
produce a distinct bit-vector; none of them alters the original set.

Using a bit-vector starts with creating one using `bitv_new()`. There are other
ways to create bit-vectors from an existing one with `bitv_union()`,
`bitv_inter()`, `bitv_minus()` and `bitv_diff()` (getting a union,
intersection, difference, symmetric difference of bit-vectors, respectively);
all bit-vector creation functions allocate storage for a bit-vector to create
and if no allocation is possible, an exception is raised instead of returning a
failure indicator like a null pointer.

Once a bit-vector created, a bit in the vector can be set and cleared using
`bitv_put()`. A sequence of bits also can be handled in group using
`bitv_set()`, `bitv_clear()`, `bitv_not()` and `bitv_setv()`; unlike a generic
set, the concept of a universe can be defined for integral sets, thus we can
have a function for complement. `bitv_get()` inspects if a certain bit is set
in a bit-vector, and `bitv_length()` gives the size (or the length) of a
bit-vector while `bitv_count()` counts the number of bits set in a given
bit-vector.

`bitv_map()` offers a way to apply some operations on every bit in a
bit-vector; it takes a user-defined function and calls it for each of bits.

`bitv_free()` takes a bit-vector (to be precise, a pointer to a bit-vector) and
releases the storage used to maintain it.


### 1.2. Boilerplate code

As an example, the following code creates two bit-vectors each of which has 60
bits. It then obtains two more from getting a union and intersection of them
after setting bits in different ways. It ends with printing bits in each set
using a user-provided function and destroying all vectors.

    int len;
    bitv_t *s, *t, *u, *v;
    unsigned char a[] = { 0x42, 0x80, 0x79, 0x29, 0x54, 0x19, 0xFF };

    s = bitv_new(60);
    t = bitv_new(60);
    len = bitv_length(s);

    bitv_set(s, 10, 50);
    bitv_setv(t, a, 7);

    u = bitv_union(s, t);
    v = bitv_inter(s, t);

    bitv_map(s, print, &len);
    bitv_map(t, print, &len);
    bitv_map(u, print, &len);
    bitv_map(v, print, &len);

    bitv_free(&s);
    bitv_free(&t);
    bitv_free(&u);
    bitv_free(&v);

where `print()` is defined as follows:

    static void print(size_t n, int v, void *cl)
    {
        printf("%s%d", (n > 0 && n % 8 == 0)? " ": "", v);
        if (n == *(int *)cl - 1)
            puts("");
    }


## 2. APIs

### 2.1. Types

#### `bit_v`

`bit_v` represents a bit-vector.

### 2.2. Creating and destroying bit-vectors

#### `bitv_t *bitv_new(size_t len)`

`bitv_new()` creates a new bit-vector.

Because a bit-vector has a much simpler data structure than a set (provided by
`cdsl/set`) does, the only information that `bitv_new()` needs to create a new
instance is the length of the bit vector it will create; `bitv_new()` will
create a bit-vector with `len` bits. The length cannot be changed after
creation.

##### May raise

`mem_exceptfail` (see the memory library from `cbl`).

##### Takes

| Name  | In/out | Meaning                        |
|:-----:|:------:|:-------------------------------|
| len   | in     | length of bit-vector to create |

##### Returns

A new bit-vector created.


#### `void bitv_free(bitv_t **pset)`

`bitv_free()` destroys a bit-vector by deallocating the storage for it and set
a given pointer to a null pointer.

##### May raise

`assert_exceptfail` (see the assertion library from `cbl`).

##### Takes

| Name  | In/out | Meaning                          |
|:-----:|:------:|:---------------------------------|
| pset  | in/out | pointer to bit-vector to destroy |

##### Returns

Nothing.


### 2.3. Handling bits in a bit-vector

#### `size_t bitv_length(const bitv_t *set)`

`bitv_length()` returns the length of a bit-vector which is the number of bits
in a bit-vector.

##### May raise

`assert_exceptfail` (see the assertion library).

##### Takes

| Name  | In/out | Meaning                                  |
|:-----:|:------:|:-----------------------------------------|
| set   | in     | bit-vector whose length will be returned |

##### Returns

The length of a bit-vector.


#### `size_t bitv_count(const bitv_t *set)`

`bitv_count()` returns the number of bits set in a bit-vector.

##### May raise

`assert_exceptfail` (see the assertion library).

##### Takes

| Name  | In/out | Meaning             |
|:-----:|:------:|:--------------------|
| set   | in     | bit-vector to count |

##### Returns

The number of bits set.


#### `int bitv_get(const bitv_t *set, size_t n)`

`bitv_get()` inspects whether a bit in a bit-vector is set or not. The position
of a bit to inspect, `n` starts at 0 and must be smaller than the length of the
bit-vector to inspect.

##### May raise

`assert_exceptfail` (see the assertion library).

##### Takes

| Name  | In/out | Meaning               |
|:-----:|:------:|:----------------------|
| set   | in     | bit-vector to inspect |
| n     | in     | bit position          |

##### Returns

A bit value (`0` or `1`).


#### `int bitv_put(bitv_t *set, size_t n, int bit)`

`bitv_put()` changes the value of a bit in a bit-vector to 0 or 1 and returns
its previous value. The position of a bit to change, `n` starts at 0 and must
be smaller than the length of the bit-vector.

##### May raise

`assert_exceptfail` (see the assertion library).

##### Takes

| Name  | In/out | Meaning           |
|:-----:|:------:|:------------------|
| set   | in/out | bit-vector to set |
| n     | in     | bit position      |
| bit   | in     | bit value         |

##### Returns

A previous value.


#### `void bitv_set(bitv_t *set, size_t l, size_t h)`

`bitv_set()` sets bits in a specified range of a bit-vector to 1.

The inclusive lower bound `l` and the inclusive upper bound `h` specify the
range. `l` must be equal to or smaller than `h`, and `h` must be smaller than
the length of the bit-vector to set.

##### May raise

`assert_exceptfail` (see the assertion library).

##### Takes

| Name  | In/out | Meaning                          |
|:-----:|:------:|:---------------------------------|
| set   | in/out | bit-vector to set                |
| l     | in     | lower bound of range (inclusive) |
| h     | in     | upper bound of range (inclusive) |

##### Returns

Nothing.


#### `void bitv_clear(bitv_t *set, size_t l, size_t h)`

`bitv_clear()` clears bits in a specified range of a bit-vector.

The inclusive lower bound `l` and the inclusive upper bound `h` specify the
range. `l` must be equal to or smaller than `h`, and `h` must be smaller than
the length of the bit-vector to clear.

##### May raise

`assert_exceptfail` (see the assertion library).

##### Takes

| Name  | In/out | Meaning                          |
|:-----:|:------:|:---------------------------------|
| set   | in/out | bit-vector to clear              |
| l     | in     | lower bound of range (inclusive) |
| h     | in     | upper bound of range (inclusive) |

##### Returns

Nothing.


#### `void bitv_not(bitv_t *set, size_t l, size_t h)`

`bitv_not()` flips bits in a specified range of a bit-vector.

The inclusive lower bound `l` and the inclusive upper bound `h` specify the
range. `l` must be equal to or smaller than `h`, and `h` must be smaller than
the length of the bit-vector to set.

##### May raise

`assert_exceptfail` (see the assertion library).

##### Takes

| Name  | In/out | Meaning                          |
|:-----:|:------:|:---------------------------------|
| set   | in/out | bit-vector to set                |
| l     | in     | lower bound of range (inclusive) |
| h     | in     | upper bound of range (inclusive) |

##### Returns

Nothing.


#### `void bitv_setv(bitv_t *set, unsigned char *v, size_t n)`

`bitv_setv()` copies bit patterns from an array of bytes to a bit vector.

Because only 8 bits in a byte are used to represent a bit-vector for
table-driven approaches, any excess bits are masked out before copying, which
explains why bitv_setv() needs to modify the array, `v`.

Be careful with how to count bit positions in a bit vector. Within a byte, the
first bit (the bit position 0) indicates the least significant bit of a byte
and the last bit (the bit position 7) does the most significant bit. Therefore,
the array

    { 0x01, 0x02, 0x03, 0x04 }

can be used to set bits of a bit-vector as follows:

    0        8        16       24     31
    |        |        |        |      |
    10000000 01000000 11000000 00100000

where the bit position is shown on the first line.

##### May raise

`assert_exceptfail` (see the assertion library).

##### Takes

| Name  | In/out | Meaning              |
|:-----:|:------:|:---------------------|
| set   | in/out | bit-vector to set    |
| v     | in/out | bit patterns to use  |
| n     | in     | size of `v` in bytes |

##### Returns

Nothing.


#### `void bitv_map(bitv_t *set, void apply(), void *cl)`

`bitv_map()` calls a user-provided function for each bit in a bit-vector. The
callback function has a type of `void (size_t, int, void *)`, and the bit
position, the bit value and the passing-by argument `cl` are passed to it.

The pointer given in `cl` is passed to the third parameter of a user callback
function, so can be used as a communication channel between the caller of
`bitv_map()` and the callback. Differently from mapping functions for other
data structures (e.g., tables and sets), changes made in an earlier invocation
to `apply()` are visible to later invocations.

##### May raise

`assert_exceptfail` (see the assertion library).

##### Takes

| Name  | In/out | Meaning                                        |
|:-----:|:------:|:-----------------------------------------------|
| set   | in/out | bit-vector with which `apply()` will be called |
| apply | in     | user-provided function (callback)              |
| cl    | in     | passing-by argument to `apply()`               |

##### Returns

Nothing.


### 2.4. Comparing bit-vectors

#### `int bitv_eq(const bitv_t *s, const bitv_t *t)`

`bitv_eq()` compares two bit-vectors to check whether they are equal. Two
bit-vectors must be of the same length.

##### May raise

`assert_exceptfail` (see the assertion library).

##### Takes

| Name  | In/out | Meaning               |
|:-----:|:------:|:----------------------|
| s     | in     | bit-vector to compare |
| t     | in     | bit-vector to compare |

##### Returns

Whether or not two bit-vectors compare equal.

| Value | Meaning   |
|:-----:|:----------|
| `0`   | not equal |
| `1`   | equal     |


#### `int bitv_leq(const bitv_t *s, const bitv_t *t)`

`bitv_leq()` compares two bit-vectors to check whether a bit-vector is a subset
of the other; note that two bit-vectors have a subset relationship for each
other when they compare equal. Two bit-vectors must be of the same length.

##### May raise

`assert_exceptfail` (see the assertion library).

##### Takes

| Name  | In/out | Meaning               |
|:-----:|:------:|:----------------------|
| s     | in     | bit-vector to compare |
| t     | in     | bit-vector to compare |

##### Returns

Whether `s` is a subset of `t`.

| Value | Meaning                    |
|:-----:|:---------------------------|
| `0`   | `s` is not a subset of `t` |
| `1`   | `s` is a subset of `t`     |


#### `int bitv_lt(const bitv_t *, const bitv_t *)`

`bitv_lt()` compares two bit-vectors to check whether a bit-vector is a proper
subset of the other. Two bit-vectors must be of the same length.

##### May raise

`assert_exceptfail` (see the assertion library).

##### Takes

| Name  | In/out | Meaning               |
|:-----:|:------:|:----------------------|
| s     | in     | bit-vector to compare |
| t     | in     | bit-vector to compare |

##### Returns

Whether `s` is a proper subset of `t`.

| Value | Meaning                           |
|:-----:|:----------------------------------|
| `0`   | `s` is not a proper subset of `t` |
| `1`   | `s` is a proper subset of `t`     |


### 2.5. Set operations

#### `bitv_t *bitv_union(const bitv_t *s, const bitv_t *t)`

`bitv_union()` creates a union of two given bit-vectors of the same length and
returns it.

One of those may be a null pointer, in which case it is considered an empty
(all-cleared) bit-vector. `bitv_union()` constitutes a distinct bit-vector from
its operands as a result, which means it always allocates storage for its
results even when one of the operands is empty. This property can be used to
make a copy of a bit-vector as follows:

    bitv_t *v *w;
    v = bitv_new(n);
    /* ... */
    w = bitv_union(v, NULL);

##### May raise

`assert_exceptfail` (see the assertion library) and `mem_exceptfail` (see the
memory library).

##### Takes

| Name  | In/out | Meaning                    |
|:-----:|:------:|:---------------------------|
| s     | in     | operand of union operation |
| t     | in     | operand of union operation |

##### Returns

The union of bit-vectors.


#### `bitv_t *bitv_inter(const bitv_t *s, const bitv_t *t)`

`bitv_inter()` creates an intersection of two bit-vectors of the same length
and returns it.

One of those may be a null pointer, in which case it is considered an empty
(all-cleared) bit-vector. `bitv_inter()` constitutes a distinct bit-vector from
its operands as a result, which means it always allocates storage for its
result even when one of the operands is empty.

##### May raise

`assert_exceptfail` (see the assertion library) and `mem_exceptfail` (see the
memory library).

##### Takes

| Name  | In/out | Meaning                           |
|:-----:|:------:|:----------------------------------|
| s     | in     | operand of intersection operation |
| t     | in     | operand of intersection operation |

##### Returns

The intersection of bit-vectors.


#### `bitv_t *bitv_minus(const bitv_t *s, const bitv_t *t)`

`bitv_minus()` returns a difference of two bit-vectors of the same length; the
bit in the resulting bit-vector is set if and only if the corresponding bit in
the first operand is set and the corresponding bit in the second operand is not
set.

One of those may be a null pointer, in which case it is considered an empty
(all-cleared) bit-vector. `bitv_minus()` constitutes a distinct bit-vector from
its operands as a result, which means it always allocates storage for its
result even when one of the operands is empty.

##### May raise

`assert_exceptfail` (see the assertion library) and `mem_exceptfail` (see the
memory library).

##### Takes

| Name  | In/out | Meaning                         |
|:-----:|:------:|:--------------------------------|
| s     | in     | operand of difference operation |
| t     | in     | operand of difference operation |

##### Returns

The difference of bit-vectors.


#### bitv_t *bitv_diff(const bitv_t *, const bitv_t *)

`bitv_diff()` returns a symmetric difference of two bit-vectors of the same
length; the bit in the resulting bit-vector is set if only one of the
corresponding bits in the operands is set.

One of those may be a null pointer, in which case it is considered an empty
(all-cleared) bit-vector. `bitv_diff()` constitutes a distinct bit-vector from
its operands as a result, which means it always allocates storage for its
result even when one of the operands is empty.

##### May raise

`assert_exceptfail` (see the assertion library) and `mem_exceptfail` (see the
memory library).

##### Takes

| Name  | In/out | Meaning                         |
|:-----:|:------:|:--------------------------------|
| s     | in     | operand of difference operation |
| t     | in     | operand of difference operation |

##### Returns

The symmetric difference of bit-vectors.


## 3. Contact me

Visit [`code.woong.org`](http://code.woong.org) to get the latest version of
this library. Any comments about the library are welcomed. If you have a
proposal or question on the library just email me, and I will reply as soon as
possible.


## 4. Copyright

For the copyright issues, see `LICENSE.md`.
