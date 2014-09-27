#define NONE

# /* ... */ if 1
Okay
# /* newline
... */ endif

#if /* newline
       ... */ 0
Nope
#else
Okay
#endif

NONE #include "dummy.c"
