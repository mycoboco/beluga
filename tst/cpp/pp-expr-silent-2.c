#define DEF defined

#if 1 || DEF DEF
#endif
#if 1 || 0x7fffffff+2    /* silent */
#endif
#if 1 || -0x7fffffff-2    /* silent */
#endif
#if 1 || 0x7fffffff*2    /* silent */
#endif
#if 1 || (-0x7fffffff-1)/-1    /* silent */
#endif
#if 1 || 1/0    /* silent */
#endif
#if 1 || -1 + 0U    /* silent */
#endif
#if 1 || 3.14
#endif
#if 1 || 0xffz
#endif
#if 1 || 0xfffffffff
#endif
#if 1 || '\xz'
#endif
#if 1 || '\xffffffff'
#endif
#if 1 || '\189'
#endif
#if 1 || '\0123'
#endif
#if 1 || '\777'
#endif
#if 1 || '\z'
#endif
#if 1 || ''
#endif
#if 1 || 'ab'
#endif
#if 1 || "abc"
#endif
#if 1 || 1+
#endif
#if 1 || 1;
#endif
#if 1 || defined(DEF
#endif
#if 1 || defined
#endif
#if 1 || defined()
#endif
#if 1 || a[0]
#endif
#if 1 || a++
#endif
#if 1 || -1U    /* silent */
#endif
#if 1 || 1U >> -1    /* silent */
#endif
#if 1 || 1U >> 100U    /* silent */
#endif
#if 1 || 1U << -1    /* silent */
#endif
#if 1 || 1U << 100U    /* silent */
#endif
#if 1 || 1 / 0U    /* silent */
#endif
#if 1 || -1 >> 1    /* silent */
#endif
#if 1 || 1 >> -1    /* silent */
#endif
#if 1 || 1 >> 100U    /* silent */
#endif
#if 1 || -1 << 1    /* silent */
#endif
#if 1 || 1 << -1    /* silent */
#endif
#if 1 || 1 << 100U    /* silent */
#endif
#if 1 || (a = 0)
#endif
#if 1 || (a += 0)
#endif
#if 1 || (0, 1)
#endif
#if 1 || (1? 0)
#endif
#if 1 || 1)
#endif
#if 1 || 1 "abc"
#endif
#if 1 || 1 abc
#endif
#if 1 || 1 @
#endif
