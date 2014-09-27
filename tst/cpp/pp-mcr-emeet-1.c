#define x f
#define f(a) a
#define exp x(42)
#define call(f) x(42)

x(42)
exp
call(x)
x(call)(f)    /* warning */

#define a(x) b
#define b(x) c
#define c(x) a

a(1)(2)(3)(4)    /* warning */
