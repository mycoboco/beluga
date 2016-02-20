void f1(void)
{
    int y, x1[10], x2[10], x3[10], x4[10];

    y = x1[0];
    x1[1] = 1[x2];
    *x3 = y;
    x1[2] = *((x3, x3)+0);
    y = y + *((x2 - x1) + x4 + (x1 - x2));
}

void f2(void)
{
    int y, x1[10] = { 0, }, x2[10] = {}, x3[10], x4[10];

    y = x1[0];
    x1[1] = 1[x2];
    *x3 = y;
    x1[2] = *((x3, x3)+0);
    y = y + *((x2 - x1) + x4 + (x1 - x2));
}

void f3(void)
{
    int y, x1[5][5], x2[5][5], x3[5][5], x4[5][5];

    y = x1[0][y];
    x1[1][1] = 1[x2][y];
    **(x3 + (&y - *x1)) = y;
    x1[2][2] = *(*((x3, x3)+y) + y);
    y = y + **((x2 - x1) + x4 + (x1 - x2));
}

void f4(void)
{
    int y;
    struct foo { int a[10]; } x1, x2, x3, g();

    y = x1.a[y];
    *(x2.a[0], (x2.a - x2.a) + x2.a) = y;
    y = y[x2.a];
    y = g().a[y];
    y = *(x3.a[y], (x2.a - x1.a) + x3.a);
}
