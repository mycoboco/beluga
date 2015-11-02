C data structure library: hash
==============================

This document specifies the hash list library which belongs to C data structure
library. The basic structure is from David Hanson's book,
[C Interfaces and Implementations](https://sites.google.com/site/cinterfacesimplementations/).
I modified the original implementation to make it more appropriate for my other
projects, to speed up operations, to add missing but useful facilities and to
enhance its readability.

The book explains its design and implementation in a very comprehensive way.
Not to mention the copyright issues, the internals of the library is not to be
explained here. Explanations for APIs, however, are given to aid the use of the
library.


## 1. Introduction

The hash library implements a
[hash table](http://en.wikipedia.org/wiki/Hash_table) and is one of the most
frequently used libraries; it is essential to get a hash key for datum before
putting it into [tables](http://en.wikipedia.org/wiki/Associative_array) by the
table library or into sets by the set library. The storage used to maintain the
hash table is managed by the library and no function in the library demands
memory allocation done by user code.

This library reserves identifiers starting with `hash_` and `HASH_`, and
imports the assertion library (which requires the exception library) and the
memory library.


### 1.1. How to use the library

The library provides one global hash table, so that there is no function that
creates a table or destroy it. A user can start to use the hash table without
its creation just by putting data to it using an appropriate function:
`hash_string()` for C strings, `hash_int()` for signed integers and
`hash_new()` for other arbitrary forms of data. Of course, because the library
internally allocates storage to manage hash keys and values, functions to
remove a certain key from the table and to completely clean up the table are
offered: `hash_free()` and `hash_reset()`.

In addition, strings are very often used to generate hash keys for them, so
`hash_vload()` and `hash_aload()` are provided and useful especially when
preloading several strings onto the table.


### 1.2. Caveats

A common mistake made when using the hash library is to pass data to functions
that expect a hash key without making one. For example, `table_put()` from the
table library requires its second argument be a hash key, but it is likely to
careless write the following code to put to a table referred to as `mytable` a
string key and its relevant value:

    char *key, *val;
    /* ... */
    table_put(mytable, key, val);

This code, however, does not work because the second argument to `table_put()`
should be a hash key not a raw string. Thus, the correct one should read as:

    table_put(mytable, hash_string(key), val);

One more thing to note is that `hash_string()` and similar functions to
generate a hash key is an actual function. If a hash key obtained from your
data is frequently used in code, it is better for efficiency to have it in a
variable rather than to call hash_string() several times.

If your compiler rejects to compile the library with a diagnostic including
`scatter[] assumes UCHAR_MAX < 256!` which says `CHAR_BIT` (the number of bits
in a byte) in your environment is larger than 8, you have to add elements to
the array `scatter` to make its total number match the number of characters in
your implementation. `scatter` is used to map a character to a random number.
For how to generate the array, see the explanation given for the array in code.


## 2. APIs

### 2.1. Creating hash strings

#### `const char *hash_string(const char *str)`

`hash_string()` returns a hash string for a given null-terminated string. It is
equivalent to call `hash_new()` with the string and its length counted by
`strlen()`.

Note that the trailing null character is not counted and it is appended when
storing into the hash table. An empty string which is consisted only of a null
character is also a valid argument for `hash_string()`; see `hash_new()` for
details.

##### May raise

`assert_exceptfail` (see the assertion library from `cbl`) and `mem_exceptfail`
(see the memory library from `cbl`).

##### Takes

| Name  | In/out | Meaning                                       |
|:-----:|:------:|:----------------------------------------------|
| str   | in     | string for which hash string will be returned |

##### Returns

A hash string for a null-terminated string.


#### `const char *hash_int(long n)`

`hash_int()` returns a hash string for a given, possibly signed, integer whose
type is `long`.

##### May raise

`mem_exceptfail` (see the memory library).

##### Takes

| Name  | In/out | Meaning                                        |
|:-----:|:------:|:-----------------------------------------------|
| n     | in     | integer for which hash string will be returned |

##### Returns

A hash string for an integer.


#### `const char *hash_new(const char *byte, size_t len)`

`hash_new()` returns a hash string for a given byte sequence, which may not end
with a null character or may have a null character embedded in it.

Even if it has "new" in its name, `hash_new()` just returns the existing hash
string if there is already one created for the same byte sequence, which means
there is only one instance of each byte sequence in the hash table; that is
what hashing is for.

An empty byte sequence which contains nothing and whose length is 0 is also
valid. But remember that `byte` has a valid pointer value by pointing to a
valid object even when `len` is zero; C allows no zero-sized object.

_It leads to an unpredictable result to modify a hash string returned by the
library._

##### May raise

`assert_exceptfail` (see the assertion library) and `mem_exceptfail` (see the
memory library).

##### Takes

| Name  | In/out | Meaning                                              |
|:-----:|:------:|:-----------------------------------------------------|
| byte  | in     | byte sequence for which hash string will be returned |
| len   | in     | length of byte sequence                              |

##### Returns

A hash string for a byte sequence.


#### `void hash_vload(const char *, ...)`

`hash_vload()` takes a possibly empty sequence of null-terminated strings and
puts them into the hash table. There should be a way to mark the end of the
argument list, which a null pointer is for.

This is useful when a program needs to preload some strings to the hash table
for later use. An array-version of `hash_vload()` is also provided; see
`hash_aload()`.

##### May raise

`mem_exceptfail` (see the memory library).

##### Takes

| Name  | In/out | Meaning                                     |
|:-----:|:------:|:--------------------------------------------|
| str   | in     | null-terminated string to put to hash table |
| ...   | in     | other such strings                          |

##### Returns

Nothing.


#### `void hash_aload(const char *[])`

`hash_aload()` takes strings from an array of strings (precisely, an array of
pointers to `char`) and puts them into the hash table. Because the function
does not take the size of the string array, there should be a way to mark end
of the array, which a null pointer is for.

This is useful when a program needs to preload some strings to the hash table
for later use. A variadic version is also provided; see `hash_vload()`.

##### May raise

`mem_exceptfail` (see the memory library).

##### Takes

| Name  | In/out | Meaning                                               |
|:-----:|:------:|:------------------------------------------------------|
| strs  | in     | array of null-terminated strings to put to hash table |

##### Returns

Nothing.


### 2.2. Destroying the hash table

#### `void hash_free(const char *)`

`hash_free()` deallocates storage for a hash string, which effectively
eliminates a hash string from the hash table. This facility is not used
frequently by user code so that the original implementation does not provide
it.

##### May raise

`assert_exceptfail` (see the assertion library).

##### Takes

| Name  | In/out | Meaning               |
|:-----:|:------:|:----------------------|
| byte  | in     | hash string to remove |

##### Returns

Nothing.


#### `void hash_reset(void)`

`hash_reset()` deallocates all hash strings in the hash table and thus resets
it.

##### May raise

Nothing.

##### Takes

Nothing.

##### Returns

Nothing.


### 2.3. Miscellaneous

#### `size_t hash_length(const char *byte)`

Given a hash string, `hash_length()` returns its length.

##### May raise

`assert_exceptfail` (see the assertion library).

##### Takes

| Name  | In/out | Meaning                                     |
|:-----:|:------:|:--------------------------------------------|
| byte  | in     | byte sequence whose length will be returned |

##### Returns

The length of a byte sequence.


## 3. Contact me

Visit [`code.woong.org`](http://code.woong.org) to get the latest version of
this library. Any comments about the library are welcomed. If you have a
proposal or question on the library just email me, and I will reply as soon as
possible.


## 4. Copyright

For the copyright issues, see `LICENSE.md`.
