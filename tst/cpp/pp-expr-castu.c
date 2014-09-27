#if -1 + 0UL
2
#endif

#define MINUS -
#define PLUS +
#define ONE 1
#if MINUS ONE PLUS 0xffffffff
9
#endif

#define X -1+0UL
#if X
14
#endif

#define F(x) -1+0 ## x
#if F    (U)
19
#endif

#if (0UL + -1) - 1 > 0
23
#endif

#if (-1 + 0UL) -1 > 0
27
#endif
