#define x1(__VA_ARGS__, __VA_ARGS__)
#define x2(__VA_ARGS__, ...)
#define x3(x) __VA_ARGS__
#define x4(...) __VA_ARGS__
#define x5 __VA_ARGS__

x1(foo, bar)
x2(foo, bar, fred)
x3()
x4(foo, bar)
x5
