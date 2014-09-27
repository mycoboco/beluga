/* -Wv -I. */

#define cmt /* */
#define empty

#define token token
#define mtoken token 123456
#define lt <
#define gt >
#define lteq <=
#define gteq >=
#define start <dummy.
#define notend c>=
#define str "string\"test"
#define long .1234567891123456789212345678931234567894123456789512345678961234567897f

#define me(q) q
#define nothing(a, b, c)

#define paste(x, y) x ## y
#define xpaste(x, y) paste(x, y)

#define L

//#include
//#include /* dummy.c */
//#include empty
//#include me(empty)
//#include nothing(1, 2, 3)
//#include me(empty) empty
//#include nothing(1, 2, 3) cmt

//#include <dummy.c> cmt
//#include <dummy.c> empty
//#include <dummy.c>cmt
//#include <dummy.c>empty
//#include "dummy.c"fred
//#include "dummy.c" fred
//#include "dummy.c"token
//#include "dummy.c" token
//#include <dummy.c>mtoken
//#include <dummy.c> mtoken

//#include cmt <dummy.c>
//#include empty <dummy.c>
//#include cmt<dummy.c>
//#include empty<dummy.c>
//#include fred"dummy.c"
//#include fred "dummy.c"
//#include token"dummy.c"
//#include token "dummy.c"
//#include mtoken<dummy.c>
//#include mtoken <dummy.c>

//#include <dummy.c>me(fred)
//#include <dummy.c>me(token)
//#include <dummy.c>me(cmt)
//#include <dummy.c>me(empty)
//#include <dummy.c>nothing(fred, 1, 2)
//#include <dummy.c>nothing(token, token, token)
//#include <dummy.c>nothing(cmt, cmt, cmt)
//#include <dummy.c>nothing(empty, empty, empty)
//#include <dummy.c>nothing
//#include <dummy.c>nothing cmt
//#include <dummy.c> me(fred)
//#include <dummy.c> me(token)
//#include <dummy.c> me(cmt)
//#include <dummy.c> me(empty)
//#include <dummy.c> nothing(fred, 1, 2)
//#include <dummy.c> nothing(token, token, token)
//#include <dummy.c> nothing(cmt, cmt, cmt)
//#include <dummy.c> nothing(empty, empty, empty)
//#include <dummy.c> nothing
//#include <dummy.c> nothing empty
//#include me(fred)<dummy.c>
//#include me(token)<dummy.c>
//#include me(cmt)<dummy.c>
//#include me(empty)<dummy.c>
//#include me<dummy.c>
//#include me empty<dummy.c>
//#include nothing(fred, 1, 2)<dummy.c>
//#include nothing(token, token, token)<dummy.c>
//#include nothing(cmt, cmt, cmt)<dummy.c>
//#include nothing(empty, empty, empty)<dummy.c>
//#include nothing <dummy.c>
//#include nothing empty <dummy.c>

//#include lt dummy.c gt
//#include lteq dummy.c gt
//#include lt dummy.c gteq
//#include start c
//#include start c gt
//#include <dummy.c gt
//#include < dummy.c gt
//#include </long/very long/x/ xpaste(long, long) gt

//#include "/etc/x>y"
#include "/etc/x\"y"
#include "/etc/x\"
#include str
#include str str
#include "string\"test"
#include "string\"test" "string\"test"
#include "string" "string"
#include L"string\"test"
#include "string" L"string"
#include L"string" "string"
#include L str

#include <dummy.c> nothing /* ... */ (
