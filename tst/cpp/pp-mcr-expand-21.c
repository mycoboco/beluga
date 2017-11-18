
/* https://groups.google.com/d/msg/comp.std.c/pex7bTpI8a8/CCa0fUoCH4UJ */

#define name2(a,b)  tmpname2(a,b)
#define tmpname2(a,b) a##b
#define declare(Class,type)  name2(Class,declare)(type)
#define MyClassdeclare(type) name2(type,MyClass)

declare(MyClass,int)


/* https://groups.google.com/d/msg/comp.std.c/bSftP3zDXLY/wzHohQ7jdFcJ */

#define b a.b
#define P(x) x
#define Q(y) P(y)

Q(z.b)


#define foobar woong
#define str(x) #x
#define con(x, y) str(x ## y)

con(foo, bar)


/* https://groups.google.com/d/msg/comp.std.c/8pX3F5h8w_E/mfoghepEVGAJ */

#define A(x) expandA()+B(x)
#define B expandB+A

B(Barg)
A(Aarg)


#define f(x) x*g
#define g f

f(2)(9)f(3)
