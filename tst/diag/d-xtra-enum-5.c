typedef enum { A } e1;
typedef enum { B } e2;

struct {
    e1 a;
} s;

void f(void)
{
    s.a = 0;
    s.a = s;
    (s.a + s.a) * s;
}

e1 f2(e1 p)
{
    if (!p)
        return p;
    else if (p == 1)
        return s;
    else
        return 0u;
}

struct { int m; } f3(e1 p)
{
    if (p)
        return (e1)0;
    else
        return p;
}

e1 *f4(e1 p)
{
    int *pi = &p;
    e1 *q = &p;

    f2(*q) * s;
    if (p)
        return q;
    else
        return p;
}

int f5(e1 p)
{
    return p;
}

void f6(e1 *p)
{
    e1 x1;
    e2 x2;
    int i;

    f5(x1);
    f5(x2);
    f5(i);
    f6(&x1);
    f6(&x2);
    f6(&i);
}
