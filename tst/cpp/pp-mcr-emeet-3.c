#define TESTCONCAT(a, b) a ## b
#define RECURSION() TESTCONCAT
#define CC RECURSION()(1,0)
#define DD RECURSION()(C,C)

DD
