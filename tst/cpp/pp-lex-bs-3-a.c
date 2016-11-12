/* -W */

#line 1 "\o"
__FILE__
#line 2 "\"
__FILE__
#line 3 "foo\@"
__FILE__
#line 4 "foo\\\""
__FILE__
#line 5 "bar\한"
__FILE__
#line 6 "bar\x글"
__FILE__
#line 7 "\7\8"
__FILE__
#line 8 "\xf\xz"
__FILE__
#line 9 "\xFF\x"
__FILE__

#define foo "\xf\xz"
#define bar "\@\"
#line 100 foo
#line 101 bar
#line 102 f\
oo

#include "pp-lex-bs-3-\@.c"
