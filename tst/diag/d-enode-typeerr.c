void f(void)
{
    int a[10], i, *p = &i;
    double d;
    struct { int m[10]; } s, g();

    a++;
    s++;
    a[3.14];
    a[a];
    a[g().m];
    a[p];
    i += 3.14;
    i += p;
    p += 3.14;
    d++;
    g().m++;
    s.m++;
}
