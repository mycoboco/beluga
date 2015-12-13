int f(void)
{
    int x1, x2, x3, y;
    struct t { int l; int m: 2; } x4, x5, x6, x7;
    struct { struct t m; } x8, x9, x10, x11, x12, f1();
    unsigned char x13, *p;

    x1 = 0;
    (x1, &x2, &x2)[&x1-&x2] = 0;
    *(double *)&x3 = 0;
    x4.l = 0;
    x6.m = 0;
    x8.m = x6;
    x9 = x8;
    x12.m.m = 0;
    x13 = f();

    y = 1 + x1;
    y = *(&x2 + x1);
    y = (&x3 + (&x2 - &x1))[x1];
    x4 = x4;
    y = x5.l;                            /* warning */
    y = x6.m;
    x7.m = y;
    x7 = x8.m;
    y = x9.m.m;
    x10.m.m = y;
    y = (&x11 + x1 + f())[1].m.m;        /* warning */
    y = (&x10 - &x9 + &x12)[f()].m.m;
    y = f1().m.m;
    f1().m.m = y;
    x1 = *(f(), p = &x13);
}
