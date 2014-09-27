char a = __FILE__ + __LINE__;

#define OK
#include "d-err-issue-2-b.c"
char *b = __FILE__[__LINE__];

#line 100 "foo"

#undef OK
#include "d-err-issue-2-b.c"
char *c = __LINE__[__FILE__];
