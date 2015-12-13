int x1; typedef double x1;                             /* error */
void f2(void) { int x2; typedef float *x2; }           /* error */
void f3(void) { typedef int x3; typedef int x3; }      /* error */
typedef int x4; void f4(void) { typedef short x4; }
void f5(void) { typedef int x5; { typedef short x5; x5 y5; } }

typedef int x1 = 0;             /* error */
typedef int x2 = { { 0, } };    /* error */
void f(typedef int *y = { { 0, } })    /* error */
{
    typedef int z1 = 0;              /* error */
    typedef int z2 = { { z1, } };    /* error */
}
void g(z) typedef int *z = { { 0, } }; {}    /* error */
