__COUNTER__
__COUNTER__

#define xpaste(a, b, c) paste(a, b, c)
#define paste(a, b, c)  a ## b ## c

xpaste(foo, __COUNTER__, bar)
xpaste(__COUNTER__, __COUNTER__, __COUNTER__)

#define __COUNTER__ foobar
__COUNTER__

#if __COUNTER__ == 7
okay
#endif
