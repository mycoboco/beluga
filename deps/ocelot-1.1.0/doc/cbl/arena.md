C basic library: arena
======================

This document specifies the arena library which belongs to C basic library. The
basic structure is from David Hanson's book,
[C Interfaces and Implementations](https://sites.google.com/site/cinterfacesimplementations/).
I modified the original implementation to make it conform to the C standard and
to enhance its readability.

The book explains its design and implementation in a very comprehensive way.
Not to mention the copyright issues, the internals of the library is not
explained here. A brief motivation, introduction and explanations for APIs,
however, are given to aid the use of the library.


## 1. Introduction

[`malloc()`](http://en.wikipedia.org/wiki/C_dynamic_memory_allocation) and
other related functions from
[`<stdlib.h>`](http://en.wikipedia.org/wiki/Stdlib.h) provide facilities for
the size-based memory allocation strategy. Each invocation to allocation
functions requires a corresponding invocation to deallocation functions in
order to avoid
[the memory leakage problem](http://en.wikipedia.org/wiki/Memory_leak). Under
certain circumstances the size-based memory allocator is not the best way to
manage storage.

For example, consider a script interpreter that accepts multiple scripts and
parses them for execution in sequence. During running a script, many data
structures including trees from a parser and tables for symbols should be
maintained and those structures often require complicated patterns of memory
allocation/deallocation. Besides, they should be correctly freed after the
script has been processed and before processing of the next script starts off.
In such a situation, a lifetime-based memory allocator can work better and
simplify the patterns of (de)allocation. With a lifetime-based memory
allocator, all storages allocated for processing a script is remembered
somehow, and all the instances can be freed at a time when processing the
script is finished. The arena library provides a lifetime-based memory
allocator.

This library reserves identifiers starting with `arena_` and `ARENA_`, and
imports the assertion library and the exception handling library. Even if it
does not depend on the memory management library, it also uses the identifier
`MEM_MAXALIGN`.


### 1.1. How to use the library

Basically, using the arena library to allocate storages does not quite differ
from using `malloc()` or similars from `<stdlib.h>`. The differences are that
every allocation function takes an additional argument to specify an arena, and
that there is no need to invoke a deallocation function for each of allocated
storages; a function to free all storages associated with an arena is provided.

    typeA *p = malloc(sizeof(*p));
    /* ... */
    typeB *q = malloc(sizeof(*p));
    /* ... */
    free(p);
    free(q);

For example, suppose that `p` and `q` point to two areas that have been
allocated at different places but can be freed at the same time. As the number
of such instances increases, deallocating them gets more complicated and thus
necessarily more error-prone.

    typeA *p = ARENA_ALLOC(myarena, sizeof(*p));
    /* ... */
    typeB *q = ARENA_ALLOC(myarena, sizeof(*q));
    /* ... */
    ARENA_FREE(myarena);

On the other hand, if an allocator that the arena library offers is used, only
one call to `ARENA_FREE()` frees all storages associated with the arena,
`myarena`.

As `<stdlib.h>` provides `malloc()` and `calloc()`, this library does
`ARENA_ALLOC()` and `ARENA_CALLOC()` taking similar arguments. `ARENA_FREE()`
does what `free()` does (actually, it does more as explained above).
`ARENA_NEW()` creates an arena with which allocated storages will be
associated, and `ARENA_DISPOSE()` destroys an arena, which means that an arena
itself may be reused repeatedly even after all storages with it have been freed
by `ARENA_FREE()`.

_As in the debugging version of the memory management library, `MEM_MAXALIGN`
indicates the common alignment factor; in other words, the alignment factor of
pointers `malloc()` returns. If it is not given, the library tries to guess a
proper value, but no guarantee that the guess is correct. Therefore, it is
recommended to give a proper definition for `MEM_MAXALIGN` (via a compiler
option like `-D`, if available)._

_Note that the arena library does not rely on the memory management library.
It constitutes a completely different allocator. Thus, the debugging version of
the memory management library cannot detect problems occurred in the storages
maintained by the arena library._


### 1.2. Boilerplate code

Using an arena starts with creating one:

    arena_t *myarena = ARENA_NEW();

As in the memory library from `cbl`, you don't need to check the return value
of `ARENA_NEW()`; an exception named `arena_exceptfailNew` will be raised if
the creation fails (see the exception library for how to handle exceptions).
Creating an arena is different from allocating a necessary storage. You can
freely allocate storages that belong to an arena with `ARENA_ALLOC()` or
`ARENA_CALLOC()` as in:

    sometype_t *p = ARENA_ALLOC(myarena, sizeof(*p));
    othertype_t *q = ARENA_CALLOC(myarena, 10, sizeof(*q));

Again, you don't have to check the return value of these invocations. If no
storage is able to be allocated, an exception, `arena_exceptfailAlloc` will be
raised. Adjusting the size of the already allocated storage is not supported;
there is no `ARENA_REALLOC()` or `ARENA_RESIZE()` that corresponds to
`realloc()`.

There are two ways to release storages from an arena: `ARENA_FREE()` and
`ARENA_DISPOSE()`.

    ARENA_FREE(myarena);
    /* myarena can be reused */
    ARENA_DISPOSE(myarena);

`ARENA_FREE()` deallocates any storage belonging to an arena, while
`ARENA_DISPOSE()` does the same job and also destroy the arena to make it no
more usable.

If you have a plan to use a tool like [Valgrind](http://valgrind.org/) to
detect memory-related bugs, see explanations for `ARENA_DISPOSE()`.


## 2. APIs

### 2.1. Types

#### `arena_t`

`arena_t` represents an arena to which storages belong.


### 2.2. Exceptions

See the exception library to see how to use exceptions.

#### `const except_t arena_exceptfailNew`

This exception is occurred when the library fails to create a new arena
probably due to memory allocation failure.

#### `const except_t arena_exceptfailAlloc`

This exception is occurred when the library fails to allocate a new storage.


### 2.3. Creating an arean

#### `arena_t *ARENA_NEW(void)`

`ARENA_NEW()` allocates a new arena and initializes it to indicate an empty
arena.

##### May raise

`arena_exceptfailNew`.

##### Takes

Nothing.

##### Returns

A new arena created


### 2.3. (De)allocating storages

#### `void *ARENA_ALLOC(arena_t *a, size_t n)`

`ARENA_ALLOC()` allocates storage whose byte length is `n` for an arena `a`.

##### May raise

`assert_exceptfail` (see the assertion library from `cbl`) and
`arena_exceptfailAlloc`.

##### Takes

| Name  | In/out | Meaning                                 |
|:-----:|:------:|:----------------------------------------|
| a     | in/out | arena for which storage to be allocated |
| n     | in     | size of storage requested in byte       |

##### Returns

A storage allocated for given arena.


#### `void *ARENA_CALLOC(arena_t *a, size_t c, size_t n)`

`ARENA_CALLOC()` allocates zero-filled storage of the size `c` * `p` for an
arena `a`.

##### May raise

`assert_exceptfail` (see the assertion library) and `arena_exceptfailAlloc`.

##### Takes

| Name  | In/out | Meaning                                 |
|:-----:|:------:|:----------------------------------------|
| a     | in/out | arena for which storage to be allocated |
| c     | in     | number of items to be allocated         |
| n     | in     | size of an item in byte                 |

##### Returns

A zero-filled storage allocated for an arena.


#### `void ARENA_FREE(arena_t *a)`

`ARENA_FREE()` deallocates all storages belonging to an arena `a`. The arena
itselt is not destroyed.

##### May raise

`assert_exceptfail` (see the assertion library).

##### Takes

| Name  | In/out | Meaning                                |
|:-----:|:------:|:---------------------------------------|
| a     | in/out | arena whose storages to be deallocated |

##### Returns

Nothing.


### 2.4. Destroying an arena

#### `void ARENA_DISPOSE(arena_t **pa)`

`ARENA_DISPOSE()` destroys an arena pointed to by `pa`. It releases storages
belonging to a given arena and disposes it. Do not be confused with
`arena_free()` which gets all storages of an arena to be deallocated but does
not destroy the arena itself.

Note that storages belonging to a free list(`freelist`, an internal list to
keep freed storages for later use) are not deallocated by `ARENA_DISPOSE()`. A
memory leakage detection tool might report that there is a leakage caused by
the library, but that is intentional, not a bug.

##### May raise

`assert_exceptfail` (see the assertion library).

##### Takes

| Name  | In/out | Meaning                     |
|:-----:|:------:|:----------------------------|
| pa    | in/out | pointer to arena to dispose |

##### Returns

Nothing.


## 3. Future directions

### 3.1. Minor changes

To mimic the behavior of `calloc()` specified by the standard, it is required
for `ARENA_CALLOC()` to successfully return a null pointer when it cannot
allocate storage of the requested size. Because this does not allow overflow,
it has to carefully check overflow when calculating the total size.


## 4. Contact me

Visit [`code.woong.org`](http://code.woong.org) to get the latest version of
this library. Any comments about the library are welcomed. If you have a
proposal or question on the library just email me, and I will reply as soon as
possible.


## 5. Copyright

For the copyright issues, see `LICENSE.md`.
