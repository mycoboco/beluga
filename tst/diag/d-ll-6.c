/* -Wv */

void f(void)
{
    float f1 = (long long)f1;
    float f2 = (unsigned long long)f2;
    double d1 = (long long)d1;
    double d2 = (unsigned long long)d2;
    long double ld1 = (long long)ld1;
    long double ld2 = (unsigned long long)ld2;

#define A1 ((long double)9223372036854775806 - 9223372036854775806.0)
#define A2 ((long double)9223372036854775807 - 9223372036854775807.0)
#define A3 ((long double)9223372036854775808 - 9223372036854775808.0)

    int a1[(A1 >= -3.0 && A1 <= 3.0)? 1: 0];
    int a2[(A2 >= -2.0 && A2 <= 2.0)? 1: 0];
    int a3[(A3 >= -1.0 && A3 <= 1.0)? 1: 0];

#define B1 ((long double)18446744073709551614 - 18446744073709551614.0)
#define B2 ((long double)18446744073709551615 - 18446744073709551615.0)
#define B3 ((long double)18446744073709551616 - 18446744073709551616.0)

    int b1[(B1 >= -3.0 && B1 <= 3.0)? 1: 0];
    int b2[(B2 >= -2.0 && B2 <= 2.0)? 1: 0];
    int b3[(B3 >= -2.0 && B3 <= 2.0)? 1: 0];
}

int x[1];
int x[(long long)1.0];
int x[(long long)2147483647.0 - 2147483646];
int x[(long long)2147483648.0 - 2147483647];
int x[(long long)4294967295.0 - 4294967294];
int x[(long long)4294967296.0 - 4294967295];
int x[(long long)9223372036854775806.0 - 9223372036854775805];
int x[(long long)9223372036854775807.0 - 9223372036854775806];
int x[(long long)9223372036854775808.0 - 9223372036854775807];

int y[(unsigned long long)18446744073709551614.0];
int y[(unsigned long long)18446744073709551614.0 - 18446744073709551613];
int y[(unsigned long long)18446744073709551615.0];
int y[(unsigned long long)18446744073709551615.0 - 18446744073709551614];
int y[(unsigned long long)18446744073709551616.0];
int y[(unsigned long long)18446744073709551616.0 - 18446744073709551615];

int z[(unsigned long long)1.0];
int z[(unsigned long long)1099511627776.0 - 1099511627775];

int w[(unsigned char)254LL];
int w[(unsigned char)0xfffffffffffffffe];
int w[(long long)(unsigned char)-1LL];
int w[(unsigned long long)(unsigned char)-1ULL];

int w[(signed char)126LL];
int w[(signed char)0xfffffffffffffffe];
int w[(long long)(signed char)126LL];
int w[(unsigned long long)(signed char)126ULL];

int v[(unsigned short)65534LL];
int v[(unsigned short)0xfffffffffffffffe];
int v[(long long)(unsigned short)-1LL];
int v[(unsigned long long)(unsigned short)-1ULL];

int v[(signed short)32766LL];
int v[(signed short)0xfffffffffffffffe];
int v[(long long)(signed short)32766LL];
int v[(unsigned long long)(signed short)32766ULL];

void g(void)
{
    char c1 = (long long)c1;
    char c2 = (unsigned long long)c2;
    unsigned char uc1 = (long long)uc1;
    unsigned char uc2 = (unsigned long long)uc2;

    short s1 = (long long)s1;
    short s2 = (unsigned long long)s2;
    unsigned short us1 = (long long)us1;
    unsigned short us2 = (unsigned long long)us1;

    int i1 = (long long)i1;
    int i2 = (unsigned long long)i2;
    unsigned u1 = (long long)u1;
    unsigned u2 = (unsigned long long)u1;

    long l1 = (long long)l1;
    long l2 = (unsigned long long)l2;
    unsigned long ul1 = (long long)ul1;
    unsigned long ul2 = (unsigned long long)ul1;

    long long ll1 = c1;
    long long ll2 = uc1;
    long long ll3 = s1;
    long long ll4 = us1;
    long long ll5 = i1;
    long long ll6 = l1;
    long long ll7 = u1;
    long long ll8 = ul1;

    unsigned long long ull1 = c1;
    unsigned long long ull2 = uc1;
    unsigned long long ull3 = s1;
    unsigned long long ull4 = us1;
    unsigned long long ull5 = i1;
    unsigned long long ull6 = l1;
    unsigned long long ull7 = u1;
    unsigned long long ull8 = ul1;

    void *p = (void *)0xffffffffffffffff;
    long long q = (long long)p;
    q = (long long)(void *)0xffffffffffffffff;
}
