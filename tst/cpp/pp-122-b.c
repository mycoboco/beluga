#define bar1
#define bar2
#undef bar2
#define bar2
#define bar2

#include "pp-122-c.c"

nothere1
#if defined nothere2
#endif
nothere3
#ifdef nothere4
#endif

#if 0
nothere5
#endif

#define nothere6 nothere6
#undef nothere7

#define fromhere
