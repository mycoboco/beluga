#define check foo == 0 && bar == 0
#if check || foobar == 0
#endif

#define foo 100
#if foo && bar == 0 || foobar == 0
#endif

#define fred 100 &&
#if fred bar == 0 || foobar == 0
#endif

#define exp(a, b) a && b
#define aaa 100
#define bbb 200
#if exp(aaa, bbb) || foobar == 0
#endif

#if exp(aaa, aaa) || foobar == 0
#endif

#define check2 foo == 0 && \
               bar == 0
#if check2 || foobar == 0
#endif
