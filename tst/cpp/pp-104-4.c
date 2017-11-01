/* -WvX */

#define foo(...) , ## __VA_ARGS__

foo()
foo(1)
foo(1,)
foo(,)
