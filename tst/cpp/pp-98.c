#define foo(x, y)


foo(quit(
#if FOO
0
  #else
1
# endif
), 0)

foo(abc
  #if FOO
0
#else
1
  #endif
,0
)

foo(
abc #if def #endif)

foo(#if)

foo( # if )
