#if 0x7fffffff+1
#endif

#if 0x7ffffffe +1 + 1
#endif

#if (1, 1+1)+0x7ffffffe
#endif

#if (0x7fffffff + 1, 0x7fffffff+1)
#endif

#if 0x7ffffffe +1 + \
1
#endif

#if 0x7ffffffe +1 \
+ 1
#endif

#if 1 / 0
#endif

#if (1+1)/(1-1)
#endif

#if (1+1)/ (1\
-1)
#endif

#if -1 < 0u
#endif

#if ((0-1) < 0 + 1u)
#endif

#define DEF def\
ined

#if defined( /* space */
#endif

#if DEF( /* space */
#endif

#if defined(foo /* space */
#endif

#if (1,1) + /* space */
#endif

#if (1,1) +/ /* space */
#endif

#if 1->mem
#endif

#if 1[2]
#endif

#if (1,1)++
#endif

#if (1,1)-\
-
#endif

#if 0u<+(1,-1)
#endif

#if -+1u
#endif

#if 0 >> 0-1
#endif

#if 0>>(0-1+0u)
#endif

#if 0<<0-1
#endif

#if 0<<(0<<(0-1+0u))-1
#endif

#if (1 && 2 && 3)+0x7fffffff
#endif

#if (1 || 2 || 3)+0x7fffffff
#endif

#if 1?-1-(1): 0u
#endif

#if 1?0u:-1-(1)
#endif

#if 1? 0u    /* space */
#endif

#if 1? 0u:    /* space */
#endif

#if 1+1)
#endif

#if 1+1 /* space */ foo
#endif
