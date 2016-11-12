/* -Wv -Db=foo(1) */

#define foo(a, b)

#define a(x, y) y

a(foo, b)
