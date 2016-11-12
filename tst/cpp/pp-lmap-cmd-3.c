/* -Wv -Dq(x)=#x */

#define f(x) q(test\)
#define b(x) x(

f(b(q))
