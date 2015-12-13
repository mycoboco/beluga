typedef signed char mychar;
typedef int myint;
typedef myint myint2;
typedef float myfloat;
typedef volatile myfloat vfloat;
typedef double mydouble;
typedef short shrt;
typedef unsigned short ushort;
typedef unsigned char uchar;
typedef char pchar;
typedef long mylong, mylong2;
typedef void myvoid;
typedef struct tag mystr, mystr2;

extern myvoid v1;
extern mystr v2;
extern mystr2 v3;

/* binary op */
void f1(void)
{
    myint x1;
    mydouble x2;

    (x1 + x2) = v2;
    (x1 - x1) = v2;
    (x2 * x2) = v2;
}

/* overflow check */
void f2(void)
{
    mychar x1 = (myint)-256;
    const mychar x2 = (const myint)-256;
    myfloat f1 = (const mydouble)1e+128;
    float f2 = (double)1e+128;
    vfloat f3 = (const mydouble)1e+128;
}

/* conversion */
void f3(void)
{
    myint x1;
    mychar x2;
    shrt x3;
    ushort x4;
    uchar x5;
    pchar x6;
    mylong x7;

    ((myint2)x1) = v2;
    x2 = v2;
    ((mydouble)x2) = v2;
    x3 = v2;
    ((myfloat)x3) = v2;
    x4 = v2;
    ((myint)x4) = v2;
    x5 = v2;
    ((mydouble)x5) = v2;
    x6 = v2;
    ((ushort)x6) = v2;
    ((mylong)x7) = v2;
    ((mylong2)x7) = v2;
}

/* assignment */
void f4(void)
{
    v1 = (void)0;
    v2 = v3;
}

/* conditional */
void f5(void)
{
    typedef myint *myptr;
    typedef const myint *mycptr;
    typedef volatile myint *myvptr;

    myint *p1;
    int *p2;
    myptr p3;
    mycptr p4;
    myvptr p5;

    ((1)? p1: p2) = v2;
    ((1)? p1: p1) = v2;
    ((1)? p1: p3) = v2;
    ((1)? p2: p3) = v2;
    ((1)? p4: p4) = v2;
    ((1)? p5: p5) = v2;
    ((1)? p4: p5) = v2;
    ((1)? p2: p5) = v2;
}

/* address */
void f6(void)
{
    typedef myvoid *vptr;
    typedef void *vptr2;
    vptr x1;
    vptr2 x2;

    &*x1;
    (&*x1) = v2;
    &*x2;
    (&*x2) = v2;
    &v1;
    (&v1) = v2;
}

/* subscript */
void f7(void)
{
    int a[10];
    char c;
    pchar d;

    a[c];
    a[d];
}

/* cast */
void f8(void)
{
    int x;

    (void)x;
    (myvoid)x;
    ((myvoid)x)++;
    ((myint)x)++;
}

/* initializer */
void f9(void)
{
    mylong x[] = L"abc";
}

/* switch */
void f10(void)
{
    myint x;
    shrt y;

    switch(x) {
        case 1: break;
    }
    switch(y) {
        case 2: break;
    }
}

/* return */
int f11(void)
{
    return (myvoid)0;
}

myvoid f12(void)
{
    return 1;
}

/* tree */
void f13(void)
{
    mystr s;
    int x;

    (1)? f12(): v1;
    ((myvoid)x);

    v1 = v1;
    s = s;
}

/* compare enum */
typedef enum { _ } myenum;
void f14(myenum);
void f14(myint x) {}

/* composite type */
myint f15(void);
myint2 f15(void);
myint2 f16(void);
myint2 f16(void);
void f17(void)
{
    void (*f)() = f15;
    void (*g)() = f16;
}
