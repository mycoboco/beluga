C data structure library: stack
===============================

This document specifies the stack library which belongs to C data structure
library. The basic structure is from David Hanson's book,
[C Interfaces and Implementations](https://sites.google.com/site/cinterfacesimplementations/).
I modified the original implementation to enhance its readability.

The book explains its design and implementation in a very comprehensive way.
Not to mention the copyright issues, the internals of the library is not to be
explained here. Explanations for APIs, however, are given to aid the use of the
library.


## 1. Introduction

The stack library is a typical implementation of a
[stack](http://en.wikipedia.org/wiki/Stack_%28abstract_data_type%29). Even if
its implementation is very similar to that of the list library, the details are
hidden behind an abstract type called `stack_t` because, unlike lists,
revealing the implementation of a stack hardly brings benefit. The storage used
to maintain a stack itself is managed by the library, but any storage allocated
for data stored in a stack should be managed by a user program.

This library reserves identifiers starting with `stack_` and `STACK_`, and
imports the assertion library (which requires the exception library) and the
memory library.


### 1.1. How to use the library

Similarly for other data structure libraries, use of the stack library follows
this sequence: create, use and destroy. If functions that allocate storage fail
memory allocation, an exception `mem_exceptfail` is raised; therefore functions
never return a null pointer.

Using a stack starts with creating it. There is only one function provided to
create a new stack, `stack_new()`. Calling it returns a new and empty stack.

Once a stack has been created, you can push data into or pop it from a stack
using `stack_push()` and `stack_pop()`, respectively. `stack_peek()` also can
be used to see what is stored at the top of a stack without popping it out.
Because popping an empty stack triggers an exception `assert_exceptfail`,
calling `stack_empty()` is recommended to inspect if a stack is empty before
applying `stack_pop()` to it.

`stack_free()` destroys a stack that is no longer necessary, but note that any
storage that is allocated by a user program does not get freed with it;
`stack_free()` only returns back the storage allocated by the library.


### 1.2. Boilerplate code

As an example, the following code creates a stack and pushes input characters
into it until `EOF` encountered. After that, it prints the characters by
popping the characters and destroy the stack.

    int c;
    char *p, *q;
    stack_t *mystack;

    mystack = stack_new();
    while ((c = getc(stdin)) != EOF) {
        MEM_NEW(p);
        *p = c;
        stack_push(mystack, p);
    }

    while (!stack_empty(mystack)) {
        p = stack_peek(mystack);
        q = stack_pop(mystack);
        assert(p == q);
        putchar(*p);
        MEM_FREE(p);
    }
    putchar('\n');

    stack_free(&mystack);

where `MEM_NEW()` and `MEM_FREE()` come from the memory library.

Note that before invoking `stack_pop()`, the stack is checked whether empty or
not by `stack_empty()` and that when popping characters, the storage allocated
for them gets freed.


## 2. APIs

### 2.1. Types

#### `stack_t`

`stack_t` represents a stack.


### 2.1. Creating and destroying stacks

#### `stack_t *stack_new(void)`

`stack_new()` creates a new stack and sets its relevant information to the
initial.

##### May raise

`mem_exceptfail` (see the memory library from `cbl`).

##### Takes

Nothing.

##### Returns

A created stack.


#### `void stack_free(stack_t **stk)`

`stack_free()` deallocates all storages for a stack and set the pointer passed
through `stk` to a null pointer.

Note that `stack_free()` does not deallocate any storage for the data in the
stack to destroy, which must be done by a user.

##### May raise

`assert_exceptfail` (see the assertion library from `cbl`).

##### Takes

| Name | In/out | Meaning                     |
|:----:|:------:|:----------------------------|
| stk  | in/out | pointer to stack to destroy |

##### Returns

Nothing.


### 2.2. Handling stacks

#### `void stack_push(stack_t *stk, void *data)`

`stack_push()` pushes data into the top of a stack.

There is no explicit limit on the maximum number of data that can be pushed
into a stack.

##### May raise

`assert_exceptfail` (see the assertion library) and `mem_exceptfail` (see the
memory library).

##### Takes

| Name | In/out | Meaning                              |
|:----:|:------:|:-------------------------------------|
| stk  | in/out | stack into which data will be pushed |
| data | in     | data to push                         |

##### Returns

Nothing.


#### `void *stack_pop(stack_t *stk)`

`stack_pop()` pops data from a stack.

If the stack is empty, an exception is raised due to the assertion failure, so
popping all data without knowing the number of nodes remained in the stack
needs to use `stack_empty()` to decide when to stop.

##### May raise

`assert_exceptfail` (see the assertion library).

##### Takes

| Name | In/out | Meaning                              |
|:----:|:------:|:-------------------------------------|
| stk  | in/out | stack from which data will be popped |

##### Returns

Data popped from the a stack.


#### `void *stack_peek(const stack_t *stk)`

`stack_peek()` provides a way to inspect the top-most data in a stack without
popping it up.

##### May raise

`assert_exceptfail` (see the assertion library).

##### Takes

| Name | In/out | Meaning       |
|:----:|:------:|:--------------|
| stk  | in     | stack to peek |

##### Returns

The top-most data in a stack.


### 2.3. Miscellaneous

#### `int stack_empty(const stack_t *)`

`stack_empty()` inspects if a stack is empty.

##### May raise

`assert_exceptfail` (see the assertion library).

##### Takes

| Name | In/out | Meaning          |
|:----:|:------:|:-----------------|
| stk  | in     | stack to inspect |

##### Returns

| Value | Meaning   |
|:-----:|:----------|
| `0`   | not empty |
| `1`   | empty     |


## 3. Contact me

Visit [`code.woong.org`](http://code.woong.org) to get the latest version of
this library. Any comments about the library are welcomed. If you have a
proposal or question on the library just email me, and I will reply as soon as
possible.


## 4. Copyright

For the copyright issues, see `LICENSE.md`.
