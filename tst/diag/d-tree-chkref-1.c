int f(void)
{
    int x1, x2, x3, y;
    struct t { int l; int m: 2; } x4, x5, x6, x7;
    struct { struct t m; } x8, x9, x10, x11, x12, f1();
    unsigned char x13, x14, *p;

    y = 1 + x1;                          /* warning */
    y = *(&x2 + x1);                     /* warning */
    y = (&x3 + (&x2 - &x1))[x1];         /* warning */
    x4 = x4;                             /* warning */
    y = x5.l;                            /* warning */
    y = x6.m;                            /* warning */
    x7.m = y;
    x7 = x8.m;                           /* warning */
    y = x9.m.m;                          /* warning */
    x10.m.m = y;
    y = (&x11 + x1 + f())[1].m.m;        /* warning */
    y = (&x10 - &x9 + &x12)[f()].m.m;    /* warning */
    y = f1().m.m;
    f1().m.m = y;
    x1 = *(f(), p = &x13);               /* warning */
    x1 = *(f(), p = &x13, p);
    x1 = *(p = &x14, x2 = x14, p);
}
