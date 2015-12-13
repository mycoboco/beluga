int f1()
{
    int *p, y, *x;
    *p = 0;          /* warning */
    y = *(x = 0);
}

int *f2()
{
    int i, j, *p = f2(&i),
              z = f1(j);      /* warning */
}

void f3(void)
{
    int y;
    struct { int x; } x1, x2, x3;
    y = x1.y;         /* warning */
    y = x2.-;
    y = x3.double;
}
