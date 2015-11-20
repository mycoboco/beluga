C data structure library: list
==============================

This document specifies the list library which belongs to C data structure
library. The basic structure is from David Hanson's book,
[C Interfaces and Implementations](https://sites.google.com/site/cinterfacesimplementations/).
I modified the original implementation to make it more appropriate for my other
projects and to enhance its readability.

The book explains its design and implementation in a very comprehensive way.
Not to mention the copyright issues, the internals of the library is not to be
explained here. Explanations for APIs, however, are given to aid the use of the
library.


## 1. Introduction

The list library is a typical implementation of a
[list](http://en.wikipedia.org/wiki/Linked_list) in which nodes have one
pointer to their next nodes; a list with two pointers to its next and previous
nodes is implemented in the doubly-linked list library. The storage used to
maintain a list itself is managed by the library, but any storage allocated for
data stored in nodes should be managed by a user program; the library provides
functions to help it.

This library reserves identifiers starting with `list_` and `LIST_`, and
imports the assertion library (which requires the exception library) and the
memory library.


### 1.1. How to use the library

Similarly for other data structure libraries, use of the list library follows
this sequence: create, use and destroy. Except for functions to inspect lists,
all other functions do one of them in various ways.

As opposed to a doubly-linked list, a list does not support random access, thus
there are facilities to aid sequential access to a list: `list_toarray()`,
`list_map()` and `LIST_FOREACH()`. These facilities help a user to convert a
list to an array, call a user-defined function for each node in a list and
traverse a list.

If functions that should allocate storage to finish their job fail the
allocation, an exception `mem_exceptfail` is raised rather than returning an
error indicator like a null pointer.

In general, pointers that library functions take and return point to
descriptors for the data structure the library implements. Once an instance of
the structure is created, the location of a descriptor for it remains unchanged
until destroyed. This property does not hold for pointers that this library
takes and returns. Those pointers in this library point to the head node of a
list rather than a descriptor for it. Because it can be replaced as a result of
operations like adding or removing a node, a user program is obliged to update
the pointer variable it passed with a returned one. Functions that accept a
list and return a modified list are `list_push()`, `list_pop()` and
`list_reverse()`.

A null pointer, which is considered invalid in other libraries, is a valid and
the only representation for an empty list. This means creating a null pointer
of the `list_t *` type in effect creates an empty list. You can freely pass it
to any functions in this library and they are guaranteed to work well with it.
Due to this, functions to add data to a list can be considered to create lists;
invoking them with a null pointer gives you a list containing the given data.
This includes `list_list()`, `list_append()`, `list_push()` and `list_copy()`.

It is considered good to hide implementation details behind an abstract type
with only interfaces exposed when designing and implementing data structures.
Exposing its implementation to users often brings nothing beneficial but
unnecessary dependency on it. In this implementation, however, the author
decided to expose its implementation because its merits triumph demerits; see
the book for more discussion on this issue.

Using a list starts with creating it. If you need just an empty list, declaring
a variable of the `list_t *` type and making it a null pointer is enough.
`list_list()`, `list_append()`, `list_push()` and `list_copy()` also create a
list by providing a null-terminated sequence of data for each node, combining
two lists, pushing a node with a given data to a list and duplicating a list.
As noted, you can use a null pointer as arguments for those functions.

Once a list has been created, a new node can be pushed (`list_push()`) and
inspected (`list_pop()`). `list_pop()` pops a node. If you need to handle a
list as if it were an array, `list_toarray()` converts a list to a
dynamically-allocated array. You can find the length of the resulting array by
calling `list_length()` or specifying a value used as a terminator (a null
pointer in most cases). `list_map()` and `LIST_FOREACH()` also provide a way to
access nodes in sequence. `list_reverse()` reverses a list, which is useful
when it is necessary to repeatedly access a list in the reverse order.

`list_free()` destroys a list that is no longer necessary, but note that any
storage that is allocated by a user program does not get freed with it;
`list_free()` only returns back the storage allocated by the library.


### 1.2. Boilerplate code

As an example, the following code creates a list and stores input characters
into each node until `EOF` encountered. After read, it prints the characters
twice by traversing the list and converting it to an array. The last input
character resides in the head node and the list behaves like a stack, which is
the reason `list_push()` and `list_pop()` are named so. The list is then
reversed and again prints the stored characters by popping nodes; after being
reversed, the order in which character are printed out differs from the former
two cases.

    int c;
    int i;
    char *p;
    void *pv, **a;
    list_t *mylist, *iter;

    mylist = NULL;
    while ((c = getc(stdin)) != EOF) {
        MEM_NEW(p);
        *p = c;
        mylist = list_push(mylist, p);
    }

    LIST_FOREACH(iter, mylist) {
        putchar(*(char *)iter->data);
    }
    putchar('\n');

    a = list_toarray(mylist, NULL);
    for (i = 0; a[i] != NULL; i++)
        putchar(*(char *)a[i]);
    putchar('\n');
    MEM_FREE(a);

    mylist = list_reverse(mylist);

    while (list_length(mylist) > 0) {
        mylist = list_pop(mylist, &pv);
        putchar(*(char *)pv);
        MEM_FREE(pv);
    }
    putchar('\n');

    list_free(&mylist);

where `MEM_NEW()` and `MEM_FREE()` come from the memory library.

In this example, the storage for each node is returned back when popping nodes
from the list. If `list_map()` were used instead to free storage, a call like
this:

    list_map(mylist, mylistfree, NULL);

would be used, where a call-back function, `mylistfree()` is defined as
follows:

    void mylistfree(void **pdata, void *ignored)
    {
        MEM_FREE(*pdata);
    }


## 2. APIs

### 2.1. Types

#### `list_t`

`list_t` represents a node in a list.

This implementation of linked lists does not employ a separate data structure
for the head or tail node; see the implementation of the doubly-linked list
library for what this means. By imposing on its users the responsibility to
make sure that a list given to the library functions is appropriate for their
tasks, it attains simpler implementation.

The detail of `list_t` is intentionally exposed to the users because doing so
is more useful. For example, a user does not need to complicate his/her code by
calling, say, `list_push()` just in order to make a temporary list node.
Declaring it as having the type of `list_t` (as opposed to `list_t *`) is
enough. In addition, an `list_t` object can be embedded in a user-created data
structure directly.

`list_t` contains the following members:

| Type              | Name  | Meaning              |
|:-----------------:|:-----:|:---------------------|
| `void *`          | data  | pointer to data      |
| `struct list_t *` | next  | pointer to next node |


### 2.2. Creating and destroying lists

#### `list_t *list_list(void *data, ...)`

`list_list()` constructs a list whose nodes contain a sequence of data given as
arguments; the first argument is stored in the head (first) node, the second
argument in the second node and so on. There should be a way to mark the end of
the argument list, which a null pointer is for. Any argument following a null
pointer argument is not invalid, but ignored.

_Calling `list_list()` with one argument, a null pointer, is not treated as an
error. Such a call requests an empty list, so returned; note that a null
pointer is an empty list._

##### May raise

`mem_exceptfail` (see the memory library from `cbl`).

##### Takes

| Name  | In/out | Meaning                                  |
|:-----:|:------:|:-----------------------------------------|
| data  | in     | data to store in head node of a new list |
| ...   | in     | other data to store in a new list        |

##### Returns

A new list containing given sequence of data.


#### `list_t *list_append(list_t *list, list_t *tail)`

`list_append()` combines two lists by appending `tail` to `list`, which makes
the next pointer of the last node in `list` point to the first node of `tail`.

_Do not forget that a null pointer is considered an empty list, not an error._

##### May raise

`mem_exceptfail` (see the memory library).

##### Takes

| Name  | In/out | Meaning                               |
|:-----:|:------:|:--------------------------------------|
| list  | in/out | list to which `tail` will be appended |
| tail  | in     | list to append to `list`              |

##### Returns

An appended list (its value is the same as `list`).


#### `list_t *list_push(list_t *list, void *data)`

`list_push()` pushes a pointer value `data` to a list `list` with creating a
new node. A null pointer given for `list` is considered to indicate an empty
list, thus not treated as an error.

_The return value of `list_push()` has to be used to update the variable for
the list passed. `list_push()` takes a list and returns a modified list._

##### May raise

`mem_exceptfail` (see the memory library).

##### Takes

| Name  | In/out | Meaning                     |
|:-----:|:------:|:----------------------------|
| list  | in     | list to which `data` pushed |
| data  | in     | data to push into `list`    |

##### Returns

A modified list


#### `list_t *list_copy(const list_t *list)`

`list_copy()` creates a new list by copying nodes of `list`.

_Note that the data pointed by nodes in `list` are not duplicated. An empty
list results in returning a null pointer, which means an empty list._

##### May raise

`mem_exceptfail` (see the memory library).

##### Takes

| Name  | In/out | Meaning           |
|:-----:|:------:|:------------------|
| list  | in     | list to duplicate |

##### Returns

A duplicated list.


#### `void list_free(list_t **plist)`

`list_free()` destroys a list by deallocating storages for each node.

After the call, the list becomes a null pointer which means it is empty. If
`plist` points to a null pointer, `list_free()` does nothing; the list is
already empty.

_Note that the type of `plist` is a double pointer to `list_t`._

##### May raise

Nothing.

##### Takes

| Name  | In/out | Meaning                    |
|:-----:|:------:|:---------------------------|
| plist | in/out | pointer to list to destroy |

##### Returns

Nothing.


### 2.3. Handling data in nodes

#### `list_t *list_pop(list_t *list, void **pdata)`

`list_pop()` copies a pointer value stored in the head node of `list` to a
pointer object pointed to by `pdata` and pops the node.

If the list is empty, `list_pop()` does nothing and just returns `list`. If
`pdata` is a null pointer, `list_pop()` just pops without saving any data.

_The return value of `list_pop()` has to be used to update the variable for the
list passed. `list_pop()` takes a list and returns a modified list._

##### May raise

Nothing.

##### Takes

| Name  | In/out | Meaning                                           |
|:-----:|:------:|:--------------------------------------------------|
| list  | in     | list from which the head node will be popped      |
| pdata | out    | pointer to pointer into which data will be stored |

##### Returns

A list with its head node popped.


#### `void **list_toarray(const list_t *list, void *end)`

`list_toarray()` converts a list to an array whose elements correspond to the
data stored in nodes of the list.

This is useful when, say, sorting a list. A function like `array_tolist()` is
not necessary. It is easy to construct a list scanning elements of an array,
for example:

    for (i = 0; i < ARRAY_SIZE; i++)
        list = list_push(list, array[i]);

The last element of the constructed array is assigned by `end`, which is a null
pointer in most cases. Do not forget to deallocate the array when it is
unnecessary.

_The size of an array generated for an empty list is not zero, because there is
always an end-mark value._

##### May raise

`mem_exceptfail` (see the memory library).

##### Takes

| Name  | In/out | Meaning                                       |
|:-----:|:------:|:----------------------------------------------|
| list  | in     | list to convert to array                      |
| end   | in     | end-mark to save in the last element of array |

##### Returns

An array converted from list.


#### `size_t list_length(const list_t *list)`

`list_length()` counts the number of nodes in `list`.

##### May raise

Nothing.

##### Takes

| Name  | In/out | Meaning                          |
|:-----:|:------:|:---------------------------------|
| list  | in     | list whose nodes will be counted |

##### Returns

The length of list.


### 2.4. Handling lists

#### `LIST_FOREACH(list_t *pos, list_t *list)`

The `LIST_FOREACH()` macro is useful when doing some task for every node of a
list. For example, the following example deallocates storages for `data` of
each node in a list named `list` and destroy `list` itself using `list_free()`:

    list_t *t;

    LIST_FOREACH(t, list)
    {
        MEM_FREE(t->data);
    }
    list_free(list);

The braces enclosing the call to `MEM_FREE` are optional in this case as you
may omit them in ordinary iterative statements. After the loop, `list` is
untouched and `t` becomes indeterminate (if the loop finishes without jumping
out of it, it should be a null pointer).

_Be warned that modification to a list like pushing and popping nodes before
finishing the loop must be done very carefully and is highly discouraged. For
example, pushing a new node with `t` may invalidate the internal state of the
list, popping a node with `list` may invalidate `t`, thus subsequent tasks
depending on `t`'s value and so on. It is possible to provide a safer version
of `LIST_FOREACH()` as done by
[Linux kernel's list implementation](https://www.kernel.org/doc/htmldocs/kernel-api/adt.html),
but not done by this implementation because such an operation is not regarded
as appropriate to the list._

##### May raise

Nothing.

##### Takes

| Name  | In/out | Meaning                   |
|:-----:|:------:|:--------------------------|
| pos   | in     | iterator to traverse list |
| list  | in     | list to traverse          |

##### Returns

Nothing; `LIST_FOREACH()` is a statement-like macro.


#### `void list_map(list_t *list, void apply(), void *cl)`

For each node in a list, list_map() calls a user-provided callback function; it
is useful when doing some common task for each node. The callback function has
a type of `void (void **, void *)`, and a pointer to data stored in each node
and the passing-by argument `cl` are passed to it.

The pointer given in `cl` is passed to the second parameter of a user callback
function, so can be used as a communication channel between the caller of
`list_map()` and the callback. Because the callback has the address of `data`
through the first parameter, it is free to change its content. One of cases
where list_map() is useful is to deallocate storages given for `data` of each
node. Differently from the original implementation, this library also provides
a macro named `LIST_FOREACH()` that can be used in similar situations.

_Be warned that modification to a list like pushing and popping nodes before
finishing `list_map()` must be done very carefully and is highly discouraged.
For example, in a callback function popping a node from the list to which
`list_map()` is applying may spoil subsequent tasks. It is possible to provide
a safer version, but not done because such an operation is not regarded as
appropriate to a list._

##### May raise

Nothing.

##### Takes

| Name  | In/out | Meaning                                  |
|:-----:|:------:|:-----------------------------------------|
| list  | in/out | list with which `apply()` will be called |
| apply | in     | user-provided function (callback)        |
| cl    | in     | passing-by argument to `apply()`         |

##### Returns

Nothing.


#### `list_t *list_reverse(list_t *list)`

`list_reverse()` reverses a list, which eventually makes its first node the
last and vice versa.

_The return value of `list_reverse()` has to be used to update the variable for
the list passed. `list_reverse()` takes a list and returns a reversed list.

##### May raise

Nothing.

##### Takes

| Name  | In/out | Meaning          |
|:-----:|:------:|:-----------------|
| list  | in/out | list to traverse |

##### Returns

A reversed list.


## 3. Future directions

### 3.1. Circular lists

Making lists circular enables appending a new node to them to be done in a
constant time. The current implementation where the last nodes point to nothing
makes `list_append()` take time proportional to the number of nodes in a list,
which is, in other words, the time complexity of `list_append()` is _O(N)_.


## 4. Contact me

Visit [`code.woong.org`](http://code.woong.org) to get the latest version of
this library. Any comments about the library are welcomed. If you have a
proposal or question on the library just email me, and I will reply as soon as
possible.


## 5. Copyright

For the copyright issues, see `LICENSE.md`.
