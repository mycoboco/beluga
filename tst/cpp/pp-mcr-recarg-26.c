#define foo(a, b)

#define a(x, y) y
#define b       foo(1)

a(foo, b)
