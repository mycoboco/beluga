int f(void)
{
    int x1 = 0, x2 = 0, x3 = 0, y;
    struct t { int l; int m: 2; } x4 = { 0 }, x5, x6 = { 0 }, x7;
    struct { struct t m; } x8 = { 0 }, x9, x10, x11, x12 = { 0 }, f1();
    unsigned char x13 = 0, *p;

    y = 1 + x1;
    y = *(&x2 + x1);
    y = (&x3 + (&x2 - &x1))[x1];
    x4 = x4;
    y = x5.l;                            /* warning */
    y = x6.m;
    x7.m = y;
    x7 = x8.m;
    y = x9.m.m;                          /* warning */
    x10.m.m = y;
    y = (&x11 + x1 + f())[1].m.m;        /* warning */
    y = (&x10 - &x9 + &x12)[f()].m.m;
    y = f1().m.m;
    f1().m.m = y;
    x1 = *(f(), p = &x13);
}
