#define line(a, b) a##b
#if 0
#line line(1,00"foo.c")
#error
#else
#line line(10,0"foo.c")
#error
#endif
