typedef void myvoid;
typedef int myint;
typedef volatile int vint;
typedef myint *ptoint;
typedef vint *ptovint;
typedef float myfloat;
typedef volatile double vdouble;
typedef struct { int m; } str;
typedef volatile struct { int m; } vstr;

myint gMyint = (myvoid *)0;
vint gCint = (myvoid *)0;
volatile myint gMyint2 = (myvoid *)0;
ptoint gPtoint = (myfloat)0.0;
ptovint gPtovint = (vdouble)0.0;
volatile ptoint gPtoint2 = (volatile myvoid *)0;
volatile ptovint gPtovint2 = (volatile vint *)0;
str gx = 0;
vstr gVstr = 0;

void f(void)
{

    myint Myint = (myvoid *)0;
    vint Vint = (myvoid *)0;
    volatile myint Myint2 = (myvoid *)0;
    ptoint Ptoint = (myfloat)0.0;
    ptovint Ptovint = (vdouble)0.0;
    volatile ptoint Ptoint2 = (volatile myvoid *)0;
    volatile ptovint Ptovint2 = (volatile vint *)0;
    str Str = 0;
    vstr Vstr = 0;

    Myint = (myvoid *)0;
    Vint = (myvoid *)0;
    Myint2 = (myvoid *)0;
    Ptoint = (myfloat)0.0;
    Ptovint = (vdouble)0.0;
    Ptoint2 = (volatile myvoid *)0;
    Ptovint2 = (volatile vint *)0;
    Str = 0;
    Vstr = 0;
}
