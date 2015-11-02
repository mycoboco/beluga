C basic library: text
=====================

This document specifies the text library which belongs to C basic library. The
basic structure is from David Hanson's book,
[C Interfaces and Implementations](https://sites.google.com/site/cinterfacesimplementations/).
I modified the original implementation to add missing but useful functions, to
make it conform to the C standard and to enhance its readability.

The book explains its design and implementation in a very comprehensive way.
Not to mention the copyright issues, the internals of the library is not to be
explained here. A simple introduction and explanations for APIs, however, are
given to aid the use of the library.


## 1. Introduction

The text library is intended to aid string manipulation in C. In C, even a
simple form of string handling like obtaining a sub-string requires lengthy
code to perform memory allocation and deallocation. This is mainly because
strings in C end with a null character, which interferes the storage for a
single string from being shared for representing its sub-strings. The text
library provides an alternative representation for strings, that is composed of
a sequence of characters (not necessarily terminated by a null) and its length
in bytes. This representation helps many string operations to be efficient. In
addition to it, the storage necessary for the strings is almost completely
controlled by the library.

This library is not intended to completely replace the C representation of
strings. To perform string operations other than those provided by the library,
a user has to convert a text back to a C string and then apply ordinary string
functions to the result. Such a conversion between those two representations is
the cost for the benefit the text library confers. To minimize the cost, some
basic text operations like comparison and mapping are also supported.

This library reserves identifiers starting with `text_` and `TEXT_`, and
imports the assertion library (which requires the exception library) and the
memory library.


### 1.1. How to use the library

For example, consider two typical cases to handle strings: obtaining a
sub-string and appending a string to another string.

    char *t;
    t = malloc(strlen(s+n) + 1);
    if (!t)
        /* ... */
    strcpy(t, s+n);
    /* ... */
    free(t);

This code shows a typical way in C to get a sub-string from a string `s` and
saves it to `t`. Because the string length is often not predictable, it is
essential for a product-level program to dynamically allocate storage for the
sub-string, and it is obliged not to forget to release it. Using the text
library, this construct changes to:

    text_t ts, tt;
    ts = text_put(s);
    tt = text_sub(ts, m, 0);

where `text_put()` converts a C string `s` to a `text_t` string `ts`, and
`text_sub()` gets a sub-string from it. `text_put()` and `text_sub()` allocate
any necessary storage and a user does not have to take care of it.

    char *s1, *s2, *t;
    t = malloc(strlen(s1)+strlen(s2) + 1);
    if (!t)
        /* ... */
    strcpy(t, s1);
    strcat(t, s2);
    /* ... */
    free(t);

Similarly, this code appends a string `s2` to another string `s1`, and saves
the result to `t`. Not to mention that the code repeats unnecessary scanning of
strings, managing storages allocated for strings is quite burdensome. Compare
this code to a version using the library:

    text_t ts1, ts2, tt;
    ts1 = text_put(s1);
    ts2 = text_put(s2);
    tt = text_cat(ts1, ts2);

All things it has to do is to convert C strings to `text_t` strings and to
apply the string concatenating operation to them.

As you can see in the examples above, there is an extra expense to convert
between C strings and `text_t` strings, but the merit that `text_t` strings
bring is quite significant and that expense is not that big if unnecessary
conversions are eliminated by a good program design.

In general, referring to a character in a string is achieved by calculating an
index of the character in the array for the string. In this library, a new and
more convenient scheme to refer to certain positions in a string is introduced;
see `text_pos()` for details. Just to mention one advantage the new scheme has,
in order to refer to the end of a string, there is no need to call a
string-length function or to inspect the `len` member of a `text_t` object;
passing `0` to a position parameter of a library function is enough.

Users are provided with partial control over the storage allocated by the
library. Managing the storage for `text_t` strings (called _the text space_) is
similar to record the state of the text space and to restore it to one of its
previous states. Whenever the library allocates storage for a string, it acts
as if it changes the state of the text space. A user code records the state
when it wants and can deallocate any storage allocated after that record by
restoring the text space to the remembered state; you might notice that the
text space behaves like a stack containing the allocated chunks. `text_save()`
and `text_restore()` explain more details.


### 1.2. Caveats

A null character that terminates a C string is not special in handling a
`text_t` string. This means that a `text_t` string can have embedded null
characters in it and all functions except for one converting to a C string
treat a null indifferently from normal characters. Note that a `text_t` string
does not need to end with a null character.

On the contrary, nothing prevents a `text_t` string from ending with a null
character. It is sometimes useful to have a `text_t` string contain a null
character, especially when conversion from a `text_t` string to a C string
occurs very frequently; note that, however, placing a null character in a
`text_t` string prohibits other strings from sharing the storage with it, which
is to give up the major advantage that the library offers.

Functions in this library always generate a new string for the result.
Comparing `strcat()` to `text_cat()` shows what this means:

    strcat(s1, s2);

Assuming that the area `s1` points to is big enough to contain the result,
`strcat()` modifies the string `s1` by appending `s2` to it. An equivalent
`text_t` version is as follows:

    t = text_cat(s1, s2);

where `t` is the resulting string and, `s1` and `s2` are left unchanged. This
difference, even if looks very small, often leads an unintended bug like
writing this kind of code:

    text_cat(s1, s2);

and expecting `s1` to point to the resulting string. The same caution goes for
`text_dup()` and `text_reverse()`, too.


### 1.3. Boilerplate code

A typical use of the text library starts with recording the state of the text
space for managing storage:

    text_save_t *chckpt;
    chckpt = text_save();

Because the state of the text space is kept before any other text library
functions are invoked, restoring the state to what is kept in `chckpt`
effectively releases all storages the text library allocates. If you don't mind
the memory leakage problem, you may ignore about saving and restoring the text
space state.

Then, the program can generate a `text_t` string from a C string (`text_box()`,
`text_put()` and `text_gen()`), convert a `text_string` back to a C string
(`text_get()`), apply various string operations (`text_sub()`, `text_cat()`,
`text_dup()` and `text_reverse()`) including mapping a string to another string
(`text_map()`), compare two strings (`text_cmp()`), locate a character in a
string (`text_chr()`, `text_rchr()`, `text_upto()`, `text_rupto()`,
`text_any()`, `text_many()` and `text_rmany()`), and locate a string in another
string (`text_find()`, `text_rfind()`, `text_match()` and `text_rmatch()`). To
aid an access to the internal of strings, `text_pos()` and `TEXT_ACCESS()` are
provided.

Finishing jobs using `text_t` strings, the following code that corresponds to
the above call to `text_save()` restores the state of the text space:

    text_restore(&chckpt);

As explained in `text_save()`, there is no requirement that `text_save()` and
its corresponding `text_restore()` be called only once; see `text_save()` and
`text_restore()` for details.


## 2. APIs

### 2.1. Types

#### `text_t`

`text_t` implements a text that is an alternative representation of character
strings.

`text_t` intentionally reveals its internals, so that a user can readily get
the length of a text and can access to the text as necessary. Modifying a text,
however, makes a program behave in an unpredictable way, which is the reason
the `str` member is `const`-qualified.

Most functions in this library take and return a `text_t` value, not a pointer
to it. This design approach simplifies an implementation because they need to
allocate no descriptor for a text. The size of `text_t` being not so big,
passing its value would cause no big penalty on performance.

| Type           | Name | Meaning                                     |
|:--------------:|:----:|:--------------------------------------------|
| `int`          | len  | length of string                            |
| `const char *` | str  | string possibly not having terminating null |


#### `text_save_t`

`text_save_t` represents information on the top of the stack-like space.

An object of the type `text_save_t` is used when remembering the current top of
the stack-like text space and restoring the space to make it have the top
remembered in `text_save_t` object as the current top. For more details, see
`text_save()` and `text_restore()`.


### 2.2. Creating texts

#### `text_t text_put(const char *str)`

`text_put()` copies a null-terminated string to the text space and returns a
text representing the copied string. The resulting text does not contain the
terminating null character. Because it always copies a given string, the
storage for the original string can be safely released after a text for it has
been generated.

##### May raise

`assert_exceptfail` (see the assertion library from `cbl`) and
`memory_exceptfail` (see the memory library from `cbl`).

##### Takes

| Type           | Name | Meaning                                                |
|:--------------:|:----:|:-------------------------------------------------------|
| `const char *` | str  | null terminated string to copy for text representation |

##### Returns

A text containing given string.


#### `text_t text_gen(const char str[], int size)`

`text_gen()` copies `size` characters from `str` to the text space and returns
a text representing the copied characters. An embedded or terminating null
character is considered an ordinary character if any.

Because it always copies given characters, the storage for the original array
can be safely released after a text for it has been generated.

`text_gen()` is useful when a caller wants to construct a text that embodies
the terminating null character with allocating storage for it. `text_put()`
allocates storage but always precludes the null character, and `text_box()` can
make the resulting text embody the null character but allocates no storage.
`text_gen()` is added to fill the gap.

##### May raise

`assert_exceptfail` (see the assertion library) and `memory_exceptfail` (see
the memory library).

##### Takes

| Type            | Name | Meaning                                    |
|:---------------:|:----:|:-------------------------------------------|
| `const char []` | str  | characters to copy for text representation |
| `int`           | size | number of characters                       |

##### Returns

A text containing given string.


#### `text_t text_box(const char *str, int len)`

`text_box()` _boxes_ a constant string or a string whose storage is already
allocated properly by a user. Unlike `text_put()`, `text_box()` does not copy a
given string and the length of a text is granted by a user. `text_box()` is
useful especially when constructing a text representation for a string literal:

    text_t t = text_box("sample", 6);

Note, in the above example, that the terminating null character is excluded by
the length given to `text_box()`. If a user gives 7 for the length, the
resulting text includes a null character, which constructs a different text
from what the above call makes.

An empty text whose length is 0 is allowed. It can be constructed simply as in
the following example:

    text_t empty = text_box("", 0);

and a predefined empty text, `text_null` is also provided for convenience.

##### May raise

`assert_exceptfail` (see the assertion library).

##### Takes

| Name  | In/out | Meaning                               |
|:-----:|:------:|:--------------------------------------|
| str   | in     | string to box for text representation |
| len   | in     | length of string to box               |

##### Returns

A text containing given string.


#### `char *text_get(char *str, int size, text_t s)`

`text_get()` converts a text to a C string. It is used when converting a text
to a C string that is null-terminated.

There are two ways to provide a buffer into which the resulting C string is to
be written. If `str` is not a null pointer, `text_get()` assumes that a user
provides the buffer whose size is `size`, and tries to write the conversion
result to it. If its specified size is not enough to contain the result, it
raises an exception due to assertion failure. If `str` is a null pointer,
`size` is ignored and `text_get()` allocates a proper buffer to contain the
resulting string. The library never deallocates the buffer allocated by
`text_get()`, thus a user has to set it free when it is no longer necessary.

##### May raise

`assert_exceptfail` (see the assertion library) and `memory_exceptfail` (see
the memory library).

##### Takes

| Name  | In/out | Meaning                                          |
|:-----:|:------:|:-------------------------------------------------|
| str   | out    | buffer into which converted string to be written |
| size  | in     | size of given buffer in bytes                    |
| s     | in     | text to convert to C string                      |

##### Returns

A pointer to buffer containing C string.


### 2.3. Text positions

#### `int text_pos(text_t s, int i)`

`text_pos()` normalizes a text position.

A text position may be negative and it is often necessary to normalize it into
the positive range. `text_pos()` takes a text position and adjusts it to the
positive range. For example, given a text:

    1  2  3  4  5  (positive positions)
      t  e  s  t
    -4 -3 -2 -1 0  (non-positive positions)
      0  1  2  3   (array indices)

both `text_pos(t, 2)` and `text_pos(t, -3)` give `2`.

##### May raise

`assert_exceptfail` (see the assertion library).

##### Takes

| Name  | In/out | Meaning                                       |
|:-----:|:------:|:----------------------------------------------|
| s     | in     | string for which position is to be normalized |
| i     | in     | position to normalize                         |

##### Returns

A nomalized positive position.


#### `char TEXT_ACCESS(text_t t, int i)`

The `TEXT_ACCESS()` macro is useful when accessing a character in a text when a
position number is known. The position can be negative, but has to be within a
valid range. For example, given 4 characters, a valid positive range for the
position is from 1 to 4 and a valid negative range from -4 to -1; 0 is never
allowed. No validity check for the range is performed.

##### May raise

Nothing.

##### Takes

| Name  | In/out | Meaning                                      |
|:-----:|:------:|:---------------------------------------------|
| t     | in     | text from which character is to be retrieved |
| i     | in     | position of character                        |

##### Returns

A character from the text.


### 2.4. Handling texts

#### `text_t text_sub(text_t s, int i, int j)`

`text_sub()` constructs a sub-text from characters between two specified
positions in a text.

Positions in a text are specified as follows:

    1  2  3  4  5  6  7    (positive positions)
      s  a  m  p  l  e
    -6 -5 -4 -3 -2 -1 0    (non-positive positions)

Given the above text, a sub-string `amp` can be specified as [2:5], [2:-2],
[-5:5] or [-5:-2]. Furthermore, the order in which the positions are given does
not matter, which means [5:2] indicates the same sequence of characters as
[2:5]. In conclusion, the following calls to text_sub() gives the same
sub-text.

    text_sub(t, 2, 5);
    text_sub(t, -5: 5);
    text_sub(t, -2: -5);
    text_sub(t, 2: -2);

A user is not allowed to modify the resulting text and it need not end with a
null character, so `text_sub()` does not have to allocate storage for the
result.

_Do not assume, however, that the resulting text always share the same storage
as the original text. An implementation might change not to guarantee it, and
there is already an exception to that assumption - when `text_sub()` returns an
empty text._

##### May raise

`assert_exceptfail` (see the assertion library).

##### Takes

| Name  | In/out | Meaning                                    |
|:-----:|:------:|:-------------------------------------------|
| s     | in     | text from which sub-text to be constructed |
| i     | in     | position for sub-text                      |
| j     | in     | position for sub-text                      |

##### Returns

A sub-text constructed.


#### `text_t text_cat(text_t s1, text_t s2)`

`text_cat()` constructs a text by concatenating two texts.

_Unlike `strcat()` in the standard library, `text_cat()` does not change a
given text by concatenation, but creates a new text by concatenating `s2` to
`s1`, which means only the returned text has the concatenated result.

##### May raise

`assert_exceptfail` (see the assertion library) and `memory_exceptfail` (see
the memory library).

##### Takes

| Name  | In/out | Meaning                                          |
|:-----:|:------:|:-------------------------------------------------|
| s1    | in     | text to which another text is to be concatenated |
| s2    | in     | text to concatenate                              |

##### Returns

A concatenated text.


#### `text_t text_dup(text_s, int n)`

`text_dup()` takes a text and constructs a text that duplicates the original
text `n` times.

For example, the following call

    text_dup(text_box("sample", 6), 3);

constructs as the result a text: `samplesamplesample`.

_Note that `text_dup()` does not change a given text, but creates a new text
that duplicates a given text. Do not forget, in this library, a text is
immutable._

##### May raise

`assert_exceptfail` (see the assertion library) and `memory_exceptfail` (see
the memory library).

##### Takes

| Name  | In/out | Meaning               |
|:-----:|:------:|:----------------------|
| s     | in     | text to duplicate     |
| n     | in     | number of duplication |

##### Returns

A text duplicated.


#### `text_t text_reverse(text_t s)`

`text_reverse` constructs a text by reversing a text.

_`text_reverse()` does not change a given text, but creates a new text by
reversing a given text._

##### May raise

`assert_exceptfail` (see the assertion library) and `memory_exceptfail` (see
the memory library).

##### Takes

| Name  | In/out | Meaning         |
|:-----:|:------:|:----------------|
| s     | in     | text to reverse |

##### Returns

A reversed text.


#### `text_t text_map(text_t s, const text_t *from, const text_t *to)`

`text_map()` constructs a text by converting a text based on a specified
mapping. It converts a text based on a mapping that is described by two
pointers to texts. Both pointers to describe a mapping should be null pointers
or non-null pointers; it is not allowed for only one of them to be a null
pointer.

When they are non-null, they should point to texts whose lengths equal.
`text_map()` takes a text and copies it converting any occurrence of characters
in a text pointed to by `from` to corresponding characters in a text pointed to
by `to`, where the corresponding characters are determined by their positions
in a text. Other characters are copied unchanged.

Once a mapping is set by calling `text_map()` with non-null text pointers,
`text_map()` can be called with a null pointers for `from` and `to`, in which
case the latest mapping is reused for conversion. Constructing a mapping table
from two texts costs. Calling with null pointers is highly recommended whenever
possible.

For example, after the following call:

    result = text_map(t, &text_upper, &text_lower);

`result` is a text copied from `t` converting any uppercase letters in it to
corresponding lowercase letters.

##### May raise

`assert_exceptfail` (see the assertion library) and `memory_exceptfail` (see
the memory library).

##### Takes

| Name  | In/out | Meaning                            |
|:-----:|:------:|:-----------------------------------|
| s     | in     | text to convert                    |
| from  | in     | pointer to text describing mapping |
| to    | in     | pointer to text describing mapping |

##### Returns

A converted text.


### 2.5. Comparing texts

#### `int text_cmp(text_t s1, text_t s2)`

`text_cmp()` compares two texts as `strcmp()` does strings except that a null
character is not treated specially.

##### May raise

`assert_exceptfail` (see the assertion library).

##### Takes

| Name  | In/out | Meaning         |
|:-----:|:------:|:----------------|
| s1    | in     | text to compare |
| s2    | in     | text to compare |

##### Returns

A comparison result.

| Value    | Meaning                        |
|:--------:|:-------------------------------|
| negative | `s1` compares less than `s2`   |
| `0`      | `s1` compares equal to `s2`    |
| positive | `s1` compares larger than `s2` |


### 2.6. Finding a character

#### `int text_chr(text_t s, int i, int j, int c)`

`text_chr()` finds the first occurrence of a character `c` in the specified
range of a text `s`. The range is specified by `i` and `j`. If found,
`text_chr()` returns the left position of the found character. It returns `0`
otherwise.

For example, given the following text:

    1  2  3  4  5  6  7    (positive positions)
      e  v  e  n  t  s
    -6 -5 -4 -3 -2 -1 0    (non-positive positions)

`text_chr(t, -6, 5, 'e')` gives `1` while `text_chr(t, -6, 5, 's')` does `0`.

##### May raise

`assert_exceptfail` (see the assertion library).

##### Takes

| Name  | In/out | Meaning                                |
|:-----:|:------:|:---------------------------------------|
| s     | in     | text in which character is to be found |
| i     | in     | range to find                          |
| j     | in     | range to find                          |
| c     | in     | character to find                      |

##### Returns

The left positive position of the found character or `0`.


#### `int text_rchr(text_t s, int i, int j, int c)`

`text_rchr()` finds the last occurrence of a character `c` in the specified
range of a text `s`. The range is specified by `i` and `j`. If found,
`text_rchr()` returns the left position of the found character. It returns `0`
otherwise.

For example, given the following text:

    1  2  3  4  5  6  7    (positive positions)
      e  v  e  n  t  s
    -6 -5 -4 -3 -2 -1 0    (non-positive positions)

`text_rchr(t, -6, 5, 'e')` gives `3` while `text_rchr(t, -6, 5, 's')` does `0`.

##### May raise

`assert_exceptfail` (see the assertion library).

##### Takes

| Name  | In/out | Meaning                                |
|:-----:|:------:|:---------------------------------------|
| s     | in     | text in which character is to be found |
| i     | in     | range to find                          |
| j     | in     | range to find                          |
| c     | in     | character to find                      |

##### Returns

The left positive position of the found character or `0`.


#### `int text_upto(text_t s, int i, int j, text_t set)`

`text_upto()` finds the first occurrence of any character from a set in a text.
The range is specified by `i` and `j`. If found, it returns the left position
of the found character. It returns `0` otherwise.

For example, given the following text:

    1  2  3  4  5  6  7    (positive positions)
      e  v  e  n  t  s
    -6 -5 -4 -3 -2 -1 0    (non-positive positions)

`text_upto(t, -6, 5, text_box("vwxyz", 5))` gives `2`. If the set containing
characters to find is empty, `text_upto()` always fails and returns `0`.

##### May raise

`assert_exceptfail` (see the assertion library).

##### Takes

| Name  | In/out | Meaning                                |
|:-----:|:------:|:---------------------------------------|
| s     | in     | text in which character is to be found |
| i     | in     | range to find                          |
| j     | in     | range to find                          |
| set   | in     | set text containing characters to find |

##### Returns

The left positive position of the found character or `0`.


#### `int text_rupto(text_t s, int i, int j, text_t set)`

`text_rupto()` finds the last occurrence of any character from a set in a text.
 The range to find is specified by `i` and `j`. If found, `text_rupto()`
returns the left position of the found character. It returns `0` otherwise.

For example, given the following text:

    1  2  3  4  5  6  7    (positive positions)
      e  v  e  n  t  s
    -6 -5 -4 -3 -2 -1 0    (non-positive positions)

`text_rupto(t, -6, 5, text_box("escape", 6))` gives `3`. If the set containing
characters to find is empty, `text_rupto()` always fails and returns `0`.

##### May raise

`assert_exceptfail` (see the assertion library).

##### Takes

| Name  | In/out | Meaning                                |
|:-----:|:------:|:---------------------------------------|
| s     | in     | text in which character is to be found |
| i     | in     | range to find                          |
| j     | in     | range to find                          |
| set   | in     | set text containing characters to find |

##### Returns

The left positive position of the found character or `0`.


#### `int text_any(text_t s, int i, text_t set)`

`text_any()` checks if a character of a specified position by `i` in a text `s`
matches any character from a set `set`. `i` specifies the left position of a
character. If it matches, it returns the right positive position of the
character or `0` otherwise.

For example, given the following text:

    1  2  3  4  5  6  7    (positive positions)
      c  a  c  a  o  s
    -6 -5 -4 -3 -2 -1 0    (non-positive positions)

`text_any(t, 2, text_box("ca", 2))` gives `3` because `a` matches. If the set
containing characters to find is empty, `text_any()` always fails and returns
`0`.

Note that giving to `i` the last position (`7` or `0` in the example text)
makes text_any() fail and return `0`; it is a valid position, so no assertion
failure.

##### May raise

`assert_exceptfail` (see the assertion library).

##### Takes

| Name  | In/out | Meaning                                |
|:-----:|:------:|:---------------------------------------|
| s     | in     | text in which character is to be found |
| i     | in     | left position of character to match    |
| set   | in     | set text containing characters to find |

##### Returns

The right positive position of the matched character or `0`.


#### `int text_many(text_s, int i, int j, text_t set)`

`text_many()` finds the end of a span consisted of characters from a set.
If the specified range of a text `s` starts with a character from a set `set`,
it returns the right positive position ending a span consisted of characters
from the set. The range is specified by `i` and `j`. It returns `0` otherwise.

For example, given the following text:

    1  2  3  4  5  6  7    (positive positions)
      c  a  c  a  o  s
    -6 -5 -4 -3 -2 -1 0    (non-positive positions)

`text_many(t, 2, 6, text_box("ca", 2))` gives `5`. If the set containing
characters to find is empty, `text_many()` always fails and returns `0`.

Because `text_many()` checks the range starts with a character from a given
set, it is often called after `text_upto()`.

##### May raise

`assert_exceptfail` (see the assertion library).

##### Takes

| Name  | In/out | Meaning                                |
|:-----:|:------:|:---------------------------------------|
| s     | in     | text in which character is to be found |
| i     | in     | range to find                          |
| j     | in     | range to find                          |
| set   | in     | set text containing characters to find |

##### Returns

The right positive position of the span or `0`.


#### `int text_rmany(text_t s, int i, int j, text_t set)`

`text_rmany()` finds the start of a span consisted of characters from a set.
If the specified range of a text `s` ends with a character from a set `set`,
it returns the left positive position starting a span consisted of characters
from the set. The range is specified by `i` and `j`. It returns `0` otherwise.

For example, given the following text:

    1  2  3  4  5  6  7    (positive positions)
      c  a  c  a  o  s
    -6 -5 -4 -3 -2 -1 0    (non-positive positions)

`text_rmany(t, 3, 7, text_box("aos", 3))` gives `4`. If the set containing
characters to find is empty, `text_rmany()` always fails and returns `0`.

Because `text_rmany()` checks the range ends with a character from a given set,
`text_rmany()` is often called after `text_rupto()`.

##### May raise

`assert_exceptfail` (see the assertion library).

##### Takes

| Name  | In/out | Meaning                                |
|:-----:|:------:|:---------------------------------------|
| s     | in     | text in which character is to be found |
| i     | in     | range to find                          |
| j     | in     | range to find                          |
| set   | in     | set text containing characters to find |

##### Returns

The right positive position of the span or `0`.


### 2.7. Finding a string

#### `int text_find(text_t s, int i, int j, text_t str)`

`text_find()` finds the first occurrence of a text `str` in the specified range
of a text `s`. The range is specified by `i` and `j`. If found, it returns the
left position of the character starting the found text. It returns `0`
otherwise.

For example, given the following text:

    1  2  3  4  5  6  7    (positive positions)
      c  a  c  a  o  s
    -6 -5 -4 -3 -2 -1 0    (non-positive positions)

`text_find(t, 6, -6, text_box("ca", 2))` gives `1`. If `str` is empty,
`text_find()` always succeeds and returns the left positive position of the
specified range.

##### May raise

`assert_exceptfail` (see the assertion library).

##### Takes

| Name  | In/out | Meaning                                   |
|:-----:|:------:|:------------------------------------------|
| s     | in     | text in which another text is to be found |
| i     | in     | range to find                             |
| j     | in     | range to find                             |
| str   | in     | text to find                              |

##### Returns

The left positive position of the found text or `0`.


#### `int text_rfind(text_t s, int i, int j, text_t str)`

`text_rfind()` finds the last occurrence of a text `str` in the specified range
of a text `s`. The range is specified by `i` and `j`. If found, `text_rfind()`
returns the left position of the character starting the found text. It returns
`0` otherwise.

For example, given the following text:

    1  2  3  4  5  6  7    (positive positions)
      c  a  c  a  o  s
    -6 -5 -4 -3 -2 -1 0    (non-positive positions)

`text_rfind(t, -6, 6, text_box("ca", 2))` gives `3`. If `str` is empty,
`text_rfind()` always succeeds and returns the right positive position of the
specified range.

##### May raise

`assert_exceptfail` (see the assertion library).

##### Takes

| Name  | In/out | Meaning                                   |
|:-----:|:------:|:------------------------------------------|
| s     | in     | text in which another text is to be found |
| i     | in     | range to find                             |
| j     | in     | range to find                             |
| str   | in     | text to find                              |

##### Returns

The left positive position of the found text or `0`.


#### `int text_match(text_t s, int i, int j, text_t str)`

`text_match()` checks if a text starts with another text. If the specified
range of a text `s` starts with a text `str`, it returns the right positive
position ending the matched text. The range is specified by `i` and `j`. It
returns `0` otherwise.

For example, given the following text:

    1  2  3  4  5  6  7    (positive positions)
      c  a  c  a  o  s
    -6 -5 -4 -3 -2 -1 0    (non-positive positions)

`text_match(t, 3, 7, text_box("ca", 2))` gives `5`. If `str` is empty,
`text_match()` always succeeds and returns the left positive position of the
specified range.

##### May raise

`assert_exceptfail` (see the assertion library).

##### Takes

| Name  | In/out | Meaning                                   |
|:-----:|:------:|:------------------------------------------|
| s     | in     | text in which another text is to be found |
| i     | in     | range to find                             |
| j     | in     | range to find                             |
| str   | in     | text to find                              |

##### Returns

The right positive position ending the matched text or `0`.


#### `int text_rmatch

`text_rmatch()` checks if a text ends with another text. If the specified range
of a text `s` ends with a text `str`, it returns the left positive position
starting the matched text. The range is specified by `i` and `j`. It returns
`0` otherwise.

For example, given the following text:

    1  2  3  4  5  6  7    (positive positions)
      c  a  c  a  o  s
    -6 -5 -4 -3 -2 -1 0    (non-positive positions)

`text_rmatch(t, 3, 7, text_box("os", 2))` gives `5`. If `str` is empty,
`text_rmatch()` always succeeds and returns the right positive position of the
specified range.

##### May raise

`assert_exceptfail` (see the assertion library).

##### Takes

| Name  | In/out | Meaning                                   |
|:-----:|:------:|:------------------------------------------|
| s     | in     | text in which another text is to be found |
| i     | in     | range to find                             |
| j     | in     | range to find                             |
| str   | in     | text to find                              |

##### Returns

The left positive position starting the matched text or `0`.


### 2.8. Text space

#### `text_save_t *text_save(void)`

`text_save()` saves the current state of the text space and returns it.

The text space to provide storages for texts can be seen as a stack and
storages allocated by `text_*()` (except that allocated by `text_get()`) can be
seen as piled up in the stack, thus any storage being used by the library after
a call to `text_save()` can be set free by calling `text_restore()` with the
saved state. After `text_restore()`, any text constructed after the call to
`text_save()` is invalidated and should never be used.

In addition, other saved states, if any, get also invalidated if the text space
gets back to a previous state by a state saved before they are generated. For
example, after the following code:

    h = text_save();
    /* ... */
    g = text_save();
    /* ... */
    text_restore(h);

The last call to `text_restore()` with `h` invalidates `g`. Calling
`text_restore()` with `g` makes the program behave in an unpredictable way.

##### May raise

`memory_exceptfail` (see the memory library).

##### Takes

Nothing.

##### Returns

A saved state of text space.


#### `void text_restore(text_save_t **save)`

`text_restore()` restores a saved state of the text space. It gets the text
space to a state returned by `text_save()`. As explained in `text_save()`, any
text and state generated after saving the state to be reverted are invalidated,
thus they should never be used.

##### May raise

`assert_exceptfail` (see the assertion library).

##### Takes

| Name  | In/out | Meaning                              |
|:-----:|:------:|:-------------------------------------|
| save  | in     | pointer to saved state of text space |

##### Returns

Nothing.


## 3. Future directions

### 3.1. Replacing stack-based storage management

The stack-like storage management by `text_save()` and `text_restore()` needs
to be replaced so that other libraries are free to use the text library. With
the current approach, invoking a clean-up function of a library that calls
`text_restore()` for the library's texts can also destroy the storage for the
program's texts. Because this effectively discourages libraries not to use the
text library, it would be better to hire a lifetime-based approach like what
used in the arena library.


### 3.2. Minor changes

If the stack-like strategy for managing the storage is not replaced as
described above, detecting some erroneous sequences of `text_save()` and
`text_restore()` would be useful. For more information, see the example given
in `text_save()`.


## 4. Contact me

Visit [`code.woong.org`](http://code.woong.org) to get the latest version of
this library. Any comments about the library are welcomed. If you have a
proposal or question on the library just email me, and I will reply as soon as
possible.


## 5. Copyright

For the copyright issues, see `LICENSE.md`.
