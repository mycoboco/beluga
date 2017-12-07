C basic library: memory
=======================

This document specifies the memory library which belongs to C basic library.
The basic structure is from David Hanson's book,
[C Interfaces and Implementations](https://sites.google.com/site/cinterfacesimplementations/).
I modified the original implementation to conform to the C standard and to
enhance its readability.

The book explains its design and implementation in a very comprehensive way.
Not to mention the copyright issues, the internals of the library is not to be
explained here. Explanations for APIs and introduction to two versions of the
library, one for production and the other for debugging, however, are given to
aid the use of the library.


## 1. Introduction

The memory library is intended to substitute calls to the memory
allocation/deallocation functions like
[`malloc()`](http://en.wikipedia.org/wiki/C_dynamic_memory_allocation) from
[`<stdlib.h>`](http://en.wikipedia.org/wiki/C_standard_library). Its main
purpose is to enhance their safety by making them:

- never return a null pointer, which eliminates handling exceptional cases
  after memory allocation; failing allocation results in raising an exception
  (that can be handled by the exception library) rather than in returning a
  null pointer, and
- set a pointer to null after freeing, which helps preventing the pointer from
  being used further.

This library reserves identifiers starting with `mem_` and `MEM_`, and imports
the assertion library and the exception library.


### 1.1. How to use the library

The following example shows a typical case to allocate/deallocate the storage
for a type that a pointer `p` points to using this library:

    {
        type_t *p;
        MEM_NEW(p);
        /* ... */
        MEM_FREE(p);
    }

The user code does not need to check the value returned from `MEM_NEW()`,
because an exception named `mem_exceptfail` raised if the allocation fails. If
you want to do something when the memory allocation fails, simply establish a
handler in a proper place as follows:

    EXCEPT_TRY
        /* code containing call to allocation functions */
    EXCEPT_EXCEPT(mem_exceptfail)
        /* code handling allocation failure */
    EXCEPT_END

`MEM_NEW0()` is also provided to do the same job as MEM_NEW() except that the
allocated storage is initialized to zero-valued bytes.

`MEM_FREE()` requires that a given pointer be an lvalue, and assigns a null
pointer to it after deallocation. This means that a user should use a temporary
object when having only a pointer value as opposed to an object containing the
value, but its benefit seems overwhelming the inconvenience.

`MEM_RESIZE()` that is intended to be a wrapper for `realloc()` differs from
`realloc()` in that its job is limited to adjusting the size of an allocated
area; `realloc()` allocates as `malloc()` when a given pointer is a null
pointer, and deallocates as free() when a given size is 0. Thus, a pointer
given to `MEM_RESIZE()` has to be non-null and a size greater than 0.

`MEM_ALLOC()` and `MEM_CALLOC()` are simple wrappers for `malloc()` and
`calloc()`, and raise an exception when the requested allocation fails.


### 1.2. Two versions

This library is served as two versions, one for production code (`memory.c`)
and the other for debugging code (`memoryd.c`). Two versions offer exactly the
same interfaces and only their implementations differ. During debugging code,
linking the debugging version is helpful when you want to figure out if there
are invalid memory usages like a free-free problem (trying to release an
already-deallocated area) and a memory leakage. This does not cover the whole
range of such problems as [valgrind](http://valgrind.org/) does, but if there
are no other tools available for catching memory problems, the debugging
version of this library would be useful. Unfortunately, the debugging version
is not able to keep track of memory usage unless allocated via this library;
for example, an invalid operation applied to the storage allocated via
`malloc()` goes undetected.


#### 1.2.1. Debugging version

As explained, the debugging version catches certain invalid memory usage. The
full list includes:

- freeing an unallocated area
- resizing an unallocated area and
- listing allocated areas at a given time.

The functions implemented in the debugging version print out no diagnostics
unless `mem_log()` is invoked properly. You can get the list of allocated areas
by calling `mem_leak()` after properly invoking `mem_log()`.


#### 1.2.2. Product version

Even if the product version does not track the memory problems that the
debugging version does, `mem_log()` and `mem_leak()` are provided as dummy
functions to let users not modify their code when switching between two
versions.


### 1.3. Caveats

In the implementation of the debugging version, `MEM_MAXALIGN` plays an
important role; it is intended to specify the alignment factor of pointers
`malloc()` returns; without that, a valid memory operation might be mistaken as
an invalid one and stop a running program issuing a wrong diagnostic message.
If `MEM_MAXALIGN` not defined, the library tries to guess a proper value, but
it is not guaranteed for the guess to be always correct. Thus, when compiling
the library, giving an explicit definition of `MEM_MAXALIGN` (via a compiler
option like `-D`, if available) is recommended.

`MEM_ALLOC()` and `MEM_CALLOC()` have the same interfaces as `malloc()` and
`calloc()` respectively, and thus their return values should be stored. On the
other hand, `MEM_NEW()` and `MEM_RESIZE()` modify a given pointer as the
result.


## 2. APIs

### 2.1 Types

#### `mem_loginfo_t`

`mem_loginfo_t` carries the information about invalid memory operations. An
object of the type `mem_loginfo_t` is used when a user-provided log function
needs the information about an invalid memory operation. As explained in
`mem_log()`, those log functions must be declared to accept a `mem_loginfo_t`
argument.

`mem_loginfo_t` contains the following members:

| Type           | Name  | Meaning                                                     |
|:--------------:|:-----:|:------------------------------------------------------------|
| `const void *` | p     | pointer value used in invalid memory operation              |
| `size_t`       | size  | requested size; meaningful when triggered by `mem_resize()` |
| `const char *` | ifile | file name in which invalid operation invoked                |
| `const char *` | ifunc | function name in which invalid operation invoked            |
| `int`          | iline | line number on which invalid operation invoked              |
| `const char *` | afile | file name in which storage in problem allocated             |
| `const char *` | afunc | function name in which storage in problem allocated         |
| `int`          | aline | line number on which storage in problem allocated           |
| `size_t`       | asize | size of storage in problem allocated before                 |

Its members contain three kinds of information:

- the information about an invalid memory operation. For example, if
  `mem_free()` is invoked for the storage that is already deallocated, the
  pointer given to `mem_free()` is passed through `p`. In case of
  `mem_resize()`, the requested size is also available in `size`.
- the information to locate an invalid memory operation. The file name,
  function name and line number where a problem occurred are provided through
  `ifile`, `ifunc` and `iline` respectively.
- the information about the memory block for which an invalid operation is
  invoked. For example, the _free-free_ case (a.k.a., _double free_) means that
  the pointer value delivered to `mem_free()` has been deallocated before.
  `afile`, `afunc`, `aline` and `asize` denote where it was allocated and what
  the size was. This information is useful in tracking how such an invalid
  operation was made.

If any of these members is not available, they are set to a null pointer (for
`ifile`, `ifunc`, `afile` and `afunc`) or `0` (for `size`, `iline`, `aline` and
`asize`).

_Logging invalid memory operations is activated by `mem_log()` which is
available only when the debugging version is used._


### 2.2. Allocating memory

#### `void *MEM_ALLOC(size_t n)`

`MEM_ALLOC()` allocates storage of the size `n` in bytes. It does the same job
as `malloc()` from `<stdlib.h>` does except:

- `MEM_ALLOC()` raises an exception(`mem_exceptfail`) when fails to allocate;
- `MEM_ALLOC()` does not take 0 as the byte length to preclude the possibility
  of returning a null pointer; and
- `MEM_ALLOC()` never returns a null pointer

##### May raise

`assert_exceptfail` (see the assertion library from `cbl`) and
`mem_exceptfail`.

##### Takes

| Name  | In/out | Meaning                                   |
|:-----:|:------:|:------------------------------------------|
| n     | in     | size in bytes for storage to be allocated |

##### Returns

A pointer to allocated storage.


#### `void *MEM_CALLOC(size_t c, size_t n)`

`MEM_CALLOC()` allocates zero-filled storage of the size `c` * `n` in bytes. It
does the same job as `MEM_ALLOC()` except that the storage it allocates are
zero-filled. The explanation similar to `MEM_ALLOC()` applies to
`MEM_CALLOC()`.

##### May raise

`assert_exceptfail` (see the assertion library) and `mem_exceptfail`.

##### Takes

| Name  | In/out | Meaning                                  |
|:-----:|:------:|:-----------------------------------------|
| c     | in     | number of items to be allocated          |
| n     | in     | size in bytes for one item               |

##### Returns

A pointer to allocated (zero-filled) storage.


#### `void MEM_NEW(p)`

The `MEM_NEW()` macro allocates to `p` storage whose size is determined by the
size of the pointed-to type by `p`.

A common way to allocate storage to a pointer `p` is as follows:

    type *p;
    p = malloc(sizeof(type));

However, this is error-prone; it might cause the memory corrupted if one
forget to change every instance of `type` when the type of `p` changes to, say,
`another_type`. To preclude problems like this a proposed way to allocate
storage for a pointer `p` is:

    p = malloc(sizeof(*p));

In this code, changing the type of `p` is automatically reflected to the
allocation code above. Note that the expression given in the `sizeof`
expression is not evaluated, so the validity of `p`'s value does not matter
here.

The macro `MEM_NEW()` facilitates such usage. It takes a pointer as an argument
and allocates to it storage whose size is the size of the referenced type.

Note that the `sizeof` operator does not evaluate its operand, which makes
`MEM_NEW()` evaluate its argument exactly once as an actual function does.
Embedding a side effect in the argument, however, is discouraged.

##### May raise

`mem_exceptfail`.

##### Takes

| Name  | In/out | Meaning                                                                         |
|:-----:|:------:|:--------------------------------------------------------------------------------|
| p     | in/out | pointer from which allocation size is determined and into which the result goes |

##### Returns

Nothing.


#### `void MEM_NEW0(p)`

The `MEM_NEW0()` macro allocates to `p` zero-filled storage whose size if
determined by the size of the pointed-to type by `p`. The only difference from
`MEM_NEW()` is that the allocated storage will be zero-filled (as allocated by
`MEM_CALLOC()`).

##### May raise

`mem_exceptfail`.

##### Takes

| Name  | In/out | Meaning                                                                         |
|:-----:|:------:|:--------------------------------------------------------------------------------|
| p     | in/out | pointer from which allocation size is determined and into which the result goes |

##### Returns

Nothing.


### 2.3. Deallocating memory

#### `void MEM_FREE(void **p)`

`MEM_FREE()` deallocates storage pointed to by `*p` (_note that the indirection
operator in front of `p`_) and set it to a null pointer.

_`p` must be a modifiable lvalue; a rvalue expression or non-modifiable lvalue
like one qualified by `const` is not allowed. Also, `MEM_FREE()` is a macro and
evaluates its argument twice, so an argument containing side effects results in
an unpredictable result._

##### May raise

Nothing.

##### Takes

| Name  | In/out | Meaning                                   |
|:-----:|:------:|:------------------------------------------|
| p     | in/out | pointer to pointer to storage to be freed |

##### Returns

Nothing.


### 2.4. Resizing memory

#### `void *MEM_RESIZE(void *p, size_t n)`

`MEM_RESIZE()` adjusts the size of storage pointed to by `p` to `n` bytes. It
does the main job of `realloc()` from `<stdlib.h>`; adjusting the size of
storage already allocated by `MEM_ALLOC()` or `MEM_CALLOC()`. While `realloc()`
deallocates like `free()` when `n` is 0 and allocates like `malloc()` when `p`
is a null pointer, `MEM_RESIZE()` accepts neither a null pointer nor zero as
its arguments. The similar explanation as for `MEM_ALLOC()` applies to
`MEM_RESIZE()`.

_`MEM_RESIZE()` is a macro and evaluates its pointer argument twice. An
argument containing side effects results in an unpredictable result._

##### May raise

`assert_exceptfail` (see the assertion library) and `mem_exceptfail`.

##### Takes

| Name  | In/out | Meaning                                       |
|:-----:|:------:|:----------------------------------------------|
| p     | in     | pointer to storage whose size to be adjusted  |
| n     | in     | new size for storage in bytes                 |

##### Returns

A pointer to size-adjusted storage.


### 2.5. Debugging memory usage

`mem_log()` and `mem_leak()` exist for debugging purpose. Only the debugging
version of the library implements them; the production version define them as
dummy functions that do nothing but immediate return to the caller.


#### `void mem_log(FILE *fp, void freefunc(), void resizefunc())`

`mem_log()` starts to log invalid memory uses; deallocating an already
released memory (_free-free_ or _double free_) and resizing a non-allocated
memory called _resize-free_. `mem_log()` provides two ways to log them. A
user can register log functions for the free-free and resize-free cases by
giving them to `freefunc` and `resizefunc` respectively. The necessary
information is provided to the registered functions via a `mem_loginfo_t`
object. A user-provided log function must be defined as follows:

    void user_freefree(FILE *fp, const mem_loginfo_t *info)
    {
        /* ... */
    }

See the explanation of `mem_loginfo_t` for the information provided to a
user-registered function. The file pointer given to `mem_log()`'s `fp` is
passed to the first parameter(`fp`) of a user-registered function.

If `freefunc` or `resizefunc` are given a null pointer, the default log
messages are printed to the file specified by `fp`. The message looks like:

    ** freeing free memory
    mem_free(0x6418) called from table_mgr() table.c:461
    this block is 48 bytes long and was allocated from table_init() table.c:233
    ** resizing unallocated memory
    mem_resize(0xf7ff, 640) called from table_mgr() table.c:468
    this block is 32 bytes long and was allocated from table_init() table.c:230

Invoking `mem_log()` with a null pointer for `fp` stops logging.

##### May raise

Nothing.

##### Takes

| Name       | In/out | Meaning                                        |
|:----------:|:------:|:-----------------------------------------------|
| fp         | in     | file to which log messages are printed out     |
| freefunc   | in     | user-provided function to log free-free case   |
| resizefunc | in     | user-provided function to log resize-free case |

##### Returns

Nothing.


#### `void mem_leak(void apply(), void *cl)`

`mem_leak()` calls a user-provided function for each of memory blocks in use.
It is useful when detecting memory leakage. Before terminating a program,
calling it with a callback function passed to `apply` makes the callback called
with the information of every memory block not deallocated yet. Among the
member of `mem_loginfo_t`, `p`, `size`, `afile`, `afunc` and `aline` are
filled; if some of them are unavailable, they are set to a null pointer for
pointer members or 0 for integer members. An information that a user needs to
give to a callback function can be passed through `cl`. The following shows an
example of a callback function:

    void inuse(const mem_loginfo_t *loginfo, void *cl)
    {
        FILE *logfile = cl;
        const char *file, func;

        file = (loginfo->afile)? loginfo->afile: "unknown file";
        func = (loginfo->afunc)? loginfo->afunc: "unknown function";

        fprintf(logfile, "** memory in use at %p\n", loginfo->p);
        fprintf(logfile, "this block is %ld bytes long and was allocated from %s() %s:%d\n",
                (unsigned long)loginfo->size, func, file, loginfo->aline);

        fflush(logfile);
    }

If this callback function is invoked as follows:

    mem_leak(inuse, stderr);

it prints out a list of memory blocks still in use to `stderr` as follows:

    ** memory in use at 0xfff7
    this block is 2048 bytes long and was allocated from table_init() table.c:235

If a null pointer is given to `apply`, the pre-defined internal callback
function is invoked to print the information for memory leak to a file given
through `cl` (after converted to `FILE *`). If `cl` is also a null pointer, a
file possibly set by `mem_log()` is inspected before the information printed
finally goes to `stderr`.

##### May raise

Nothing.

##### Takes

| Name       | In/out | Meaning                                                 |
|:----------:|:------:|:--------------------------------------------------------|
| apply      | in     | user-provided function for each of memory blocks in use |
| freefunc   | in     | pass-by argument to `apply`                             |

##### Returns

Nothing.


## 3. Future directions

### 3.1. Minor changes

To mimic the behavior of `calloc()` specified by the standard, it is required
for the debugging version of `MEM_CALLOC()` to successfully return a null
pointer when it cannot allocate the storage of the requested size. Because this
does not allow overflow, it has to carefully check overflow when calculating
the total size.


## 4. Contact me

Visit [`code.woong.org`](http://code.woong.org) to get the latest version of
this library. Any comments about the library are welcomed. If you have a
proposal or question on the library just email me, and I will reply as soon as
possible.


## 5. Copyright

For the copyright issues, see `LICENSE.md`.
