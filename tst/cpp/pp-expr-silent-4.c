#define DEF defined

#if 0 && DEF DEF
#endif
#if 0 && 0x7fffffff+2
#endif
#if 0 && -0x7fffffff-2
#endif
#if 0 && 0x7fffffff*2
#endif
#if 0 && (-0x7fffffff-1)/-1
#endif
#if 0 && 1/0
#endif
#if 0 && -1 + 0U
#endif
#if 0 && 3.14
#endif
#if 0 && 0xffz
#endif
#if 0 && 0xfffffffff
#endif
#if 0 && '\xz'
#endif
#if 0 && '\xffffffff'
#endif
#if 0 && '\189'
#endif
#if 0 && '\0123'
#endif
#if 0 && '\777'
#endif
#if 0 && '\z'
#endif
#if 0 && ''
#endif
#if 0 && 'ab'
#endif
#if 0 && "abc"
#endif
#if 0 && 1+
#endif
#if 0 && 1;
#endif
#if 0 && defined(DEF
#endif
#if 0 && defined
#endif
#if 0 && defined()
#endif
#if 0 && a[0]
#endif
#if 0 && a++
#endif
#if 0 && -1U
#endif
#if 0 && 1U >> -1
#endif
#if 0 && 1U >> 100U
#endif
#if 0 && 1U << -1
#endif
#if 0 && 1U << 100U
#endif
#if 0 && 1 / 0U
#endif
#if 0 && -1 >> 1
#endif
#if 0 && 1 >> -1
#endif
#if 0 && 1 >> 100U
#endif
#if 0 && -1 << 1
#endif
#if 0 && 1 << -1
#endif
#if 0 && 1 << 100U
#endif
#if 0 && (a = 0)
#endif
#if 0 && (a += 0)
#endif
#if 0 && (0, 1)
#endif
#if 0 && (1? 0)
#endif
#if 0 && 1)
#endif
#if 0 && 1 "abc"
#endif
#if 0 && 1 abc
#endif
#if 0 && 1 @
#endif
