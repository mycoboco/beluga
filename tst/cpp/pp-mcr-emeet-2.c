#define f(x) g(x)
#define g(x) f(

g(2)9)    /* warning */

#undef g
#define g(a) a*f

g(2)(9)    /* warning */

#define m n
#define n(a) a

m(m)
