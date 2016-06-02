void f(void)
{
    int q;
    foo_t x;
    bar_t *p;

    x++;
    *p++;
    x.m;
    p->m;

    (int)x;
    (void)*p;
    (void *)p;

    --x;
    --p;
    +x;
    -*p;
    !*p;
    ~x;
    *x;
    &x;
    sizeof(x);
    sizeof(*p);

    x * p;
    *p + x;
    x << *p;
    p >> x;

    *p < x;
    p != x;

    *p & x;
    p | x;

    *p && x;
    p || x;

    q = (p)? *p: x;
    q = *p;
    x = *p;
    *p += x;
    p <<= x;

    *p, x, *p;

    if (*p);
    if (p);
    for (; x; x++);
}

void f1(foo_t);
void f2(foo_t x, foo_t y);

void g(void)
{
    foo_t *q;

    f1(0);
    f2(0);
    f2(0, 0);
    f2(0, 0, 0);
    f1(*q);
    f2(q, *q);
}
