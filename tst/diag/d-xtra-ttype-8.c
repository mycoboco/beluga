typedef struct inc inc;
typedef volatile struct inc2 inc2;
typedef void myvoid;
typedef const void cvoid;

inc Inc = { 0 };
inc2 Inc2 = { 0 };
myvoid Myvoid = 0;
cvoid Cvoid = 0;

void f1(void)
{
    inc Inc = { 0 };
    inc2 Inc2 = { 0 };
}


typedef struct { int m; } str;
typedef volatile struct { int m; } vstr;
typedef int aint[3];
typedef volatile int avint[3];

str Str = 0;
vstr Vstr = 0;
aint Aint = 0;
avint Avint = 0;

void f2(void)
{
    str Str = 0;
    vstr Vstr = 0;
    aint Aint = 0;
    avint Avint = 0;
}


typedef char mychar;
typedef volatile mychar vchar;
typedef mychar achar[3];
typedef vchar avchar[3];
typedef int myint;
typedef volatile int vint;

achar Achar = "abcd";
avchar Avchar = "abcd";
achar Achar2 = { 'a', 'b', 'c', 'd' };
avchar Avchar2 = { 'a', 'b', 'c', 'd' };
aint Aint2 = { 0, 1, 2, 3 };
avint Avint2 = { 0, 1, 2, 3 };
str Str2 = { 0, 1 };
vstr Vstr2 = { 0, 1 };

void f3(void)
{
    achar Achar = "abcd";
    avchar Avchar = "abcd";
    achar Achar2 = { 'a', 'b', 'c', 'd' };
    avchar Avchar2 = { 'a', 'b', 'c', 'd' };
    aint Aint2 = { 0, 1, 2, 3 };
    avint Avint2 = { 0, 1, 2, 3 };
    str Str2 = { 0, 1 };
    vstr Vstr2 = { 0, 1 };
}
