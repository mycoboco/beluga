typedef signed char schar;
typedef volatile vschar;
typedef unsigned char uchar;
typedef volatile vuchar;

typedef int myint;
typedef volatile myint vint;
typedef long mylong;
typedef volatile mylong vlong;

typedef unsigned myunsigned;
typedef volatile myunsigned vunsigned;
typedef unsigned long ulong;
typedef volatile ulong vulong;

typedef float myfloat;
typedef double mydouble;
typedef long double ldouble;
typedef volatile myfloat vfloat;
typedef volatile mydouble vdouble;
typedef volatile ldouble vldouble;

typedef struct { int m; } str;

typedef void myvoid;
typedef volatile myvoid *pvoid;

/* from fp */
void f1(void)
{
    str s;
    vfloat vf;
    vdouble vd;
    vldouble vld;

    (vschar)vf * s;
    (vuchar)vf * s;
    (vint)vf * s;
    (vlong)vf * s;
    (vunsigned)vf * s;
    (vulong)vf * s;

    (vschar)vd * s;
    (vuchar)vd * s;
    (vint)vd * s;
    (vlong)vd * s;
    (vunsigned)vd * s;
    (vulong)vd * s;

    (vschar)vld * s;
    (vuchar)vld * s;
    (vint)vld * s;
    (vlong)vld * s;
    (vunsigned)vld * s;
    (vulong)vld * s;

    (vfloat)vf * s;
    (vdouble)vd * s;
    (vldouble)vld * s;

    (vfloat)vd * s;
    (vdouble)vf * s;
    (vldouble)vf * s;
}

/* from integer */
void f2(void)
{
    str s;
    vschar vsc;
    vuchar vuc;
    vint vi;
    vlong vl;
    vunsigned vu;
    vulong vul;
    pvoid pv;

    (vfloat)vsc * s;
    (vdouble)vsc * s;
    (vldouble)vsc * s;

    (vfloat)vuc * s;
    (vdouble)vuc * s;
    (vldouble)vuc * s;

    (vfloat)vi * s;
    (vdouble)vi * s;
    (vldouble)vi * s;

    (vfloat)vl * s;
    (vdouble)vl * s;
    (vldouble)vl * s;

    (vfloat)vu * s;
    (vdouble)vu * s;
    (vldouble)vu * s;

    (vfloat)vul * s;
    (vdouble)vul * s;
    (vldouble)vul * s;

    (vschar)vsc * s;
    (vuchar)vuc * s;
    (vint)vi * s;
    (vlong)vl * s;
    (vunsigned)vu * s;
    (vulong)vul * s;

    (vschar)vi * s;
    (vlong)vul * s;
    (vunsigned)vl * s;

    (pvoid)vsc;
    (pvoid)vuc;
    (pvoid)vi;
    (pvoid)vl;
    (pvoid)vu;
    (pvoid)vul;

    (vschar)pv;
    (vuchar)pv;
    (vint)pv;
    (vunsigned)pv;
    (vlong)pv;
    (vulong)pv;
}
