#define f(x) q(test\)
#define b(x) x(
#define q(x) #x

f(b(q))
