#define foo() bar
#define proc(x, y) x ## #y

proc(foo, bar)
