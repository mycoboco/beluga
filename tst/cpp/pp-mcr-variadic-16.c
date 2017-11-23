/* -WXv */

#define foo(a, ...) a ## , ## __VA_ARGS__

foo()
foo(a)
foo(a,)
foo(a,b)
foo(a,b,c)
