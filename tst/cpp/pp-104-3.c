/* -WvX */

#define foo(a, ...) a , ## __VA_ARGS__

foo(1)
foo(1, )
foo(1, 2)
foo(1, , 2)
