int x1; typedef double x1;
void f2(void) { int x2; typedef float *x2; }
void f3(void) { typedef int x3; typedef int x3; }
typedef int x4; void f4(void) { typedef short x4; }
void f5(void) { typedef int x5; { typedef short x5; x5 y5; } }

typedef int x1 = 0;
typedef int x2 = { { 0, } };
void f(typedef int *y = { { 0, } })
{
    typedef int z1 = 0;
    typedef int z2 = { { z1, } };
}
void g(z) typedef int *z = { { 0, } }; {}
