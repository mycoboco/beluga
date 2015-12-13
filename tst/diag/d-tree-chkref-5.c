int f(void)
{
    static int x1, x2, x3, y;
    static struct t { int l; int m: 2; } x4, x5, x6, x7;
    static struct s { struct t m; } x8, x9, x10, x11, x12;
    struct s f1();
    static unsigned char x13, *p;

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
    f1().m.m = y;
    x1 = *(f(), p = &x13);
}
