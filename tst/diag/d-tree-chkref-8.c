int f(int *p)
{
    int *g();
    int y, x1, x2, x3, x4, x5, x6, x7, a[10];

    p = &x1;
    y = x1;

    f(&x2);
    y = x2;

    f1(a);
    y = a[0];

    y = *g(&x3);
    y = x3;

    y = *((g())? &x4: &x5);

    y = (g(&x6, &x7))? x6: x7;
}
