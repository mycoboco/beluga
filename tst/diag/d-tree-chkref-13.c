int f1()
{
    int *p, y, *x;
    *p = 0;
    y = *(x = 0);
}

int *f2()
{
    int i, j, *p = f2(&i),
              z = f1(j);
}

void f3(void)
{
    int y;
    struct { int x; } x1, x2, x3;
    y = x1.y;
    y = x2.-;
    y = x3.double;
}
