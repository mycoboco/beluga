int f(void)
{
    int x1, x2, x3, y;
    struct t { int l; int m: 2; } x4, x5, x6, x7;
    struct { struct t m; } x8, x9, x10, x11, x12, f1();
    unsigned char x13, x14, *p;

    y = 1 + x1;
    y = *(&x2 + x1);
    y = (&x3 + (&x2 - &x1))[x1];
    x4 = x4;
    y = x5.l;
    y = x6.m;
    x7.m = y;
    x7 = x8.m;
    y = x9.m.m;
    x10.m.m = y;
    y = (&x11 + x1 + f())[1].m.m;
    y = (&x10 - &x9 + &x12)[f()].m.m;
    y = f1().m.m;
    x1 = *(f(), p = &x13);
    x1 = *(f(), p = &x13, p);
    x1 = *(p = &x14, x2 = x14, p);
}

void g(void)
{
    int y;
    struct t { int l; int m: 2; };
    struct { struct t m; } f1();

    f1().m.m = y;
}
