typedef struct tag str;
typedef volatile struct tag vstr;

str f1(void);
vstr f2(void);

void g(void)
{
    f1();
    f2();
}


typedef int myint;
typedef volatile int vint;
typedef float myfloat;
typedef myint *pint;
typedef vint *pvint;

void g2(void)
{
    myint Myint;
    vint Vint;
    myfloat Myfloat;
    pint Pint;
    pvint Pvint;
    str Str;
    vstr Vstr;

    void h(str, str, vstr, vstr, str, myint);
    h(Myint, Vint, Myfloat, Pint, Pvint, Vstr);
    h2(Str, Vstr);
}
