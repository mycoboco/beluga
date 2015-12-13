
/* qualifier under INDIR */
typedef volatile int vint;

void f1(void)
{
    vint x, *px;
    x = f1();
    *px = f1();
}

/* incomplete array */
typedef int incarr[];
extern incarr x1 = { 1, 2 };

void f2(void)
{
    sizeof(x1);
}

extern incarr x2;

void f3(void)
{
    sizeof(x2);
}

/* incomplete struct */
typedef struct tag incstr;
extern incstr x3;

void f4(void)
{
    x3.m;
    sizeof(x3);
}

struct tag { int m; };
extern incstr x4;

void f5(void)
{
    x4.m;
    sizeof(x4);
}
