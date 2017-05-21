C data structure library: doubly-linked list
============================================

This document specifies the doubly-linked list library which belongs to C data
structure library. The basic structure is from David Hanson's book,
[C Interfaces and Implementations](https://sites.google.com/site/cinterfacesimplementations/).
I modified the original implementation to make it more appropriate for my other
projects, to speed up operations and to enhance its readability.

The book explains its design and implementation in a very comprehensive way.
Not to mention the copyright issues, the internals of the library is not to be
explained here. Explanations for APIs, however, are given to aid the use of the
library.


## 1. Introduction

The doubly-linked list library is a typical implementation of a
[doubly-linked list](http://en.wikipedia.org/wiki/Doubly_linked_list) in which
nodes have two pointers to their next and previous nodes; a list with a
unidirectional pointer
(a [singly-linked list](http://en.wikipedia.org/wiki/Linked_list)) is
implemented in the list library. The storage used to maintain a list itself is
managed by the library, but any storage allocated for data stored in nodes
should be managed by a user program.

This library reserves identifiers starting with `dlist_` and `DLIST_`, and
imports the assertion library (which requires the exception library) and the
memory library.


### 1.1. How to use the library

Similarly to other data structure libraries, a typical use of the doubly-linked
list library follows this sequence: create, use and destroy. Except for
functions to inspect lists, all other functions do one of them in various ways.

As opposed to a singly-linked list, a doubly-linked list enables its nodes to
be accessed randomly. To speed up such accesses, the library is revised from
the original version so that a list remembers which node was last accessed. If
a request is made to access a node that is next or previous to the remembered
node, the library locates it starting from the remembered node. This is from
observation that traversing a list from the head or the tail in sequence occurs
frequently in many programs and can make a program making heavy use of lists
run almost 3 times faster. Therefore, for good performance of your program, it
is highly recommended that lists are traversed sequentially whenever possible.
Do not forget that the current implementation requires the library to locate
the desired node from the head or the tail for other types of accesses (that
is, any access to a node that is not immediately next or previous to a
remembered node).

Using a list starts with creating one. The simplest way to do it is to call
`dlist_new()`. `dlist_new()` returns an empty list, and if it fails to allocate
storage for the list, an exception `mem_exceptfail` is raised rather than
returning a null pointer. All functions that allocate storage signal a shortage
of memory via the exception; no null pointer returned. There is another
function to create a list: `dlist_list()` that accepts a sequence of data and
creates a list containing them in each node.

Once a list has been created, a new node can be inserted in various ways
(`dlist_add()`, `dlist_addhead()` and `dlist_addtail()`) and an existing node
can be removed from a list also in various ways (`dlist_remove()`,
`dlist_remhead()` and `dlist_remtail()`). You can inspect the data of a node
(`dlist_get()`) or replace it with new one (`dlist_put()`). In addition, you
can find the number of nodes in a list (`dlist_length()`) or can rotate (or
shift) a list (`dlist_shift()`). For an indexing scheme used when referring to
existing nodes, see `dlist_get()`. For that used when referring to a position
into which a new node inserted, see `dlist_add()`.

`dlist_free()` destroys a list that is no longer necessary, but note that any
storage that is allocated by a user program does not get freed with it;
`dlist_free()` only returns back the storage allocated by the library.


### 1.2. Boilerplate code

As an example, the following code creates a list and stores input characters
into each node until `EOF` encountered. After read, it copies characters in
nodes to continuous storage area to construct a string and prints the string.

    int c;
    char *p, *q;
    dlist_t *mylist;

    mylist = dlist_new();

    while ((c = getc(stdin)) != EOF) {
        MEM_NEW(p);
        *p = c;
        dlist_addtail(mylist, p);
    }

    n = dlist_length(mylist);

    p = MEM_ALLOC(n+1);
    for (i = 0; i < n; i++) {
        p = dlist_get(mylist, i);
        q[i] = *p;
        MEM_FREE(p);
    }
    q[i] = '\0';

    dlist_free(&mylist);

    puts(q);

where `MEM_NEW()`, `MEM_ALLOC()` and `MEM_FREE()` come from the memory library.

Note that, before adding a node to a list, unique storage to contain a
character is allocated with `MEM_NEW()` and this storage is returned back by
`MEM_FREE()` while copying characters into an allocated array.


## 2. APIs

### 2.1. Types

#### `dlist_t`

`dlist_t` represents a doubly-linked list.


### 2.2. Creating and destroying lists

#### `dlist_t *dlist_new(void)`

`dlist_new()` creates an empty list and returns it for further use.

##### May raise

`mem_exceptfail` (see the memory library from `cbl`).

##### Takes

Nothing.

##### Returns

An empty new list.


#### `dlist_t *dlist_list(void *data, ...)`

`dlist_list()` constructs a doubly-linked list whose nodes contain a sequence
of data given as arguments. The first argument is stored in the head (first)
node, the second argument in the second node and so on.

There should be a way to mark the end of the argument list, which a null
pointer is for. Any argument following a null pointer argument is not invalid,
but ignored.

_Calling `dlist_list()` with one argument, a null pointer, is not treated as an
error. Such a call request an empty list like calling `dlist_new()`._

##### May raise

`mem_exceptfail` (see the memory library).

##### Takes

| Name  | In/out | Meaning                                |
|:-----:|:------:|:---------------------------------------|
| data  | in     | data to store in head node of new list |
| ...   | in     | other data to store in new list        |

##### Returns

A new list containing a given sequence of data.


#### `void dlist_free(dlist_t **pdlist)`

`dlist_free()` destroys a list by deallocating storages for each node and for
the list itself.

After a call to `dlist_free()`, the list does not exist (do not be confused by
a non-existing list with an existing but empty list). If `pdlist` points to a
null pointer, an assertion in `dlist_free()` fails.

##### May raise

`assert_exceptfail` (see the assertion library from `cbl`).

##### Takes

| Name   | In/out | Meaning                    |
|:------:|:------:|:---------------------------|
| pdlist | in/out | pointer to list to destroy |

##### Returns

Nothing.


### 2.3. Adding and removing nodes

#### `void *dlist_add(dlist_t *dlist, long pos, void *data)`

`dlist_add()` inserts a new node to a position specified by `pos`.

The position is interpreted as follows: (5 nodes assumed to be in a list)

     1   2    3    4    5    6    positive position values
      +-+  +-+  +-+  +-+  +-+
      | |--| |--| |--| |--| |
      +-+  +-+  +-+  +-+  +-+
    -5   -4   -3   -2   -1   0    non-positive position values

Non-positive positions are useful when to locate without knowing the length of
a list. If a list is empty both 0 and 1 are the valid values for a new node.
Note that `pos - (dlist_length()+1)` gives a non-negative value for a positive
`pos`, and `pos + (dlist_length()+1)` gives a positive value for a negative
`pos`.

##### May raise

`assert_exceptfail` (see the assertion library) and `mem_exceptfail` (see the
memory library).

##### Takes

| Name  | In/out | Meaning                                 |
|:-----:|:------:|:----------------------------------------|
| dlist | in/out | list to which new node will be inserted |
| pos   | in     | position for new node                   |
| data  | in     | data for new node                       |

##### Returns

Data for a new node.


#### `void *dlist_addhead(dlist_t *dlist, void *data)`

`dlist_addhead()` inserts a new node before the head node; the new node will be
the head node. `dlist_addhead()` is equivalent to dlist_add() with `1` given
for the position.

##### May raise

`assert_exceptfail` (see the assertion library) and `mem_exceptfail` (see the
memory library).

##### Takes

| Name  | In/out | Meaning                                 |
|:-----:|:------:|:----------------------------------------|
| dlist | in/out | list to which new node will be inserted |
| data  | in     | data for new node                       |

##### Returns

Data for a new node.


#### `void *dlist_addtail(dlist_t *dlist, void *data)`

`dlist_addtail()` inserts a new node after the last node; the index for the new
node will be N if there are N nodes before the insertion. If a list is empty,
`dlist_addtail()` and `dlist_addhead()` do the same job. `dlist_addtail()` is
equivalent to `dlist_add()` with `0` given for the position.

##### May raise

`assert_exceptfail` (see the assertion library) and `mem_exceptfail` (see the
memory library).

##### Takes

| Name  | In/out | Meaning                                 |
|:-----:|:------:|:----------------------------------------|
| dlist | in/out | list to which new node will be inserted |
| data  | in     | data for new node                       |

##### Returns

Data for a new node.


#### `void *dlist_remove(dlist_t *dlist, long i)`

`dlist_remove()` removes the `i`-th node from a list. For indexing, see
`dlist_get()`.

##### May raise

`assert_exceptfail` (see the assertion library).

##### Takes

| Name  | In/out | Meaning                              |
|:-----:|:------:|:-------------------------------------|
| dlist | in/out | list from which node will be removed |
| i     | in     | index for node to remove             |

##### Returns

Data of the removed node.


#### `void *dlist_remhead(dlist_t *dlist)`

`dlist_remhead()` removes the first (head) node from a list. `dlist_remhead()`
is equivalent to `dlist_remove()` with `0` for the position.

##### May raise

`assert_exceptfail` (see the assertion library).

##### Takes

| Name  | In/out | Meaning                                        |
|:-----:|:------:|:-----------------------------------------------|
| dlist | in/out | list from which the first node will be removed |

##### Returns

Data of the removed node.


#### `void *dlist_remtail(dlist_t *dlist)`

`dlist_remtail()` removes the last (tail) node of a list. `dlist_remtail()` is
equivalent to `dlist_remove()` with `dlist_length()-1` for the index.

##### May raise

`assert_exceptfail` (see the assertion library).

##### Takes

| Name  | In/out | Meaning                                       |
|:-----:|:------:|:----------------------------------------------|
| dlist | in/out | list from which the last node will be removed |

##### Returns

Data of the removed node.


### 2.4. Handling lists

#### `long dlist_length(const dlist_t *dlist)`

`dlist_length()` returns the length of a list, the number of nodes in it.

##### May raise

`assert_exceptfail` (see the assertion library).

##### Takes

| Name  | In/out | Meaning                            |
|:-----:|:------:|:-----------------------------------|
| dlist | in     | list whose length will be returned |

##### Returns

The length of a list (non-negative).


#### `void *dlist_get(dlist_t *dlist, long i)`

`dlist_get()` brings and return data in the `i`-th node in a list. The first
node has the index 0 and the last has _n_-1 when there are _n_ nodes in a list.

##### May raise

`assert_exceptfail` (see the assertion library).

##### Takes

| Name  | In/out | Meaning                                |
|:-----:|:------:|:---------------------------------------|
| dlist | in/out | list from which data will be retrieved |
| i     | in     | index for node                         |

##### Returns

Data retrieved from a node.


#### `void *dlist_put(dlist_t *dlist, long i, void *data)`

`dlist_put()` replaces the data stored in the `i`-th node with new given data
and retrieves the old data. For indexing, see `dlist_get()`.

##### May raise

`assert_exceptfail` (see the assertion library).

##### Takes

| Name  | In/out | Meaning                          |
|:-----:|:------:|:---------------------------------|
| dlist | in/out | list whose data will be replaced |
| i     | in     | index for node                   |
| data  | in     | new data for substitution        |

##### Returns

Old data stored in a node.


#### `void dlist_shift(dlist_t *dlist, long n)`

`dlist_shift()` shifts a list to right or left according to the value of `n`.

A positive value indicates shift to right; for example shift by 1 means to make
the tail node become the head node. Similarly, a negative value indicates shift
to left; for example shift by -1 means to make the head node become the tail
node.

The absolute value of the shift distance specified by `n` should be equal to or
less than the length of a list. For exmple, `dlist_shift(..., 7)` or
`dlist_shift(..., -7)` is not allowed when there are only 6 nodes in a list. In
such a case, `dlist_shift(..., 6)` or `dlist_shift(..., -6)` has no effect as
`dlist_shift(..., 0)`.

Note that it is a list itself that `dlist_shift()` shifts, not the head pointer
of a list.

##### May raise

`assert_exceptfail` (see the assertion library).

##### Takes

| Name  | In/out | Meaning                         |
|:-----:|:------:|:--------------------------------|
| dlist | in/out | list to shift                   |
| n     | in     | direction and distance of shift |

##### Returns

Nothing.


## 3. Contact me

Visit [`code.woong.org`](http://code.woong.org) to get the latest version of
this library. Any comments about the library are welcomed. If you have a
proposal or question on the library just email me, and I will reply as soon as
possible.


## 4. Copyright

For the copyright issues, see `LICENSE.md`.
