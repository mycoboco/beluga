void f1()
{
    int g(double *);
    int *h(double *);
    int y, x1, x2, x3, x4, x5, x6;
    struct { int x; } x7;

    y = x1 + y + &x1;
    y = &x1 + f1(x2);                 /* warning */
    y = x1 + g(x3);                   /* warning */
    y = x1 + h(x4);                   /* warning */
    y = x5.m;
    y = ((struct { int x; })x6).x;
    y = x1 + x7;
}

int f2()
{
    int y1, y2, x1[10], x2, x3[10], *x4, x5, x6, x7[10], x8;
    struct { int x; } s;

    if (y1[x1] = x2);                /* warning */
    if (y2[x3] = x4);
    switch((s+y1).x) { case 0:; }
    return x7[x8] = x5(&x6) + x6;
}
