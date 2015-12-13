typedef void void1, void2;

void f1(void)
{
    extern void1 Void1;
    (void2)Void1;
}

typedef int int1, int2;

void f2(void)
{
    int1 Int1;
    (int2)Int1;
}

typedef unsigned unsigned1;
typedef unsigned int unsigned2;

void f3(void)
{
    unsigned1 Unsigned1;
    (unsigned2)Unsigned1;
}

typedef long long1;
typedef long int long2;

void f4(void)
{
    long1 Long1;
    (long2)Long1;
}

typedef unsigned long ulong1;
typedef unsigned long int ulong2;

void f5(void)
{
    ulong1 Ulong1;
    (ulong2)Ulong1;
}

typedef float float1, float2;

void f6(void)
{
    float1 Float1;
    (float2)Float1;
}

typedef double double1, double2;

void f7(void)
{
    double1 Double1;
    (double2)Double1;
}

typedef long double ldouble1, ldouble2;

void f8(void)
{
    ldouble1 Ldouble1;
    (ldouble2)Ldouble1;
}

typedef char pchar1, pchar2;
typedef signed char schar1, schar2;
typedef unsigned char uchar1, uchar2;

void f9(void)
{
    pchar1 Pchar1;
    schar1 Schar1;
    uchar1 Uchar1;

    (pchar2)Pchar1;
    (schar2)Schar1;
    (uchar2)Uchar1;
}

typedef short short1, short2;
typedef unsigned short ushort1, ushort2;

void f10(void)
{
    short2 Short2;
    ushort1 Ushort1;

    (short1)Short2;
    (ushort2)Ushort1;
}

typedef enum { A } enum11, enum12;
typedef enum { B } enum2;

void f11(void)
{
    enum11 Enum11;

    (enum12)Enum11;
    (enum2)Enum11;
}

typedef struct { int m; } str11, str12;
typedef struct { int m; } str2;

void f12(void)    /* not test ty_same() */
{
    extern str11 s;
    extern str12 s;
    extern str2 s;
}

typedef int1 *ptoint1;
typedef int2 *ptoint2;

void f13(void)
{
    ptoint1 Ptoint1;
    (ptoint2)Ptoint1;
}

typedef const int cint1;
typedef const int2 cint2;

void f14(void)
{
    cint1 Cint1;
    (cint2)Cint1;
}

typedef int1 aint1[];
typedef int2 aint2[10];
typedef int aint3[3];

void f15(void)    /* not test ty_same() */
{
    extern aint1 Aint;
    extern aint2 Aint;
    extern aint3 Aint;
}

typedef int1 ft1(int1, int2);
typedef int ft2(int2, int1);
typedef int1 ft3();
typedef int1 ft4(void);
typedef int1 ft5(int1, int2, ...);

void f16(void)    /* not test ty_same() */
{
    extern ft1 F1;
    extern ft2 F1;
    extern ft3 F1;
    extern ft4 F1;
    extern ft5 F1;
}
