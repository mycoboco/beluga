#define DEF defined
#if DEF (
#endif

#if DEF
#endif

#define BIG 0x7fffffff
#if 1 + BIG
#endif

#define BIGP 0x7fffffff +
#if BIGP 1
#endif
