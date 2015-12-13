void f(void)
{
    int a[10], i, *p = &i;
    double d;
    struct { int m[10]; } s, g();

    a++;          /* error */
    s++;          /* error */
    a[3.14];      /* error */
    a[a];         /* error */
    a[g().m];     /* error */
    a[p];         /* error */
    i += 3.14;
    i += p;       /* error */
    p += 3.14;    /* error */
    d++;
    g().m++;      /* error */
    s.m++;        /* error */
}
