foo1_t *p, **pp;

void f(void)
{
    foo2_t * const q = pp;
    const foo3_t *q2 = p;

    p + pp;
    q = p;
    q2 = q2 - p;
    q2();
}

foo1_t (*q)[];
foo1_t (*q)[10];

void g(void)
{
    q = g;
}

foo3_t (*q)[];
foo3_t (*r)[];
