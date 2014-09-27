#define x defined
#define y x
#if x
#endif

#if x x
7
#endif
#if x y
10
#endif

#define a defined a
#if a
15
#endif
#if a x
18
#endif

#if x a
22
#endif
