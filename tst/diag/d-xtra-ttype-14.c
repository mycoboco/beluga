typedef enum { A } myenum;
typedef myenum *penum;
typedef myenum * volatile vpenum;
typedef int myint;
typedef myint *pint;

void f(void)
{
    penum p;
    pint q;
    penum *pp;
    pint *qq;
    vpenum r;

    p = q;
    pp = qq;
    r = q;
    r = pp;
}
