foo1_t a1[10];
foo2_t a2[];
foo3_t a3[10][];

void f(void)
{
    a1 = a1[0];
    a2 = a2[0];
    a3 = a3[0];

    sizeof(a1);
    sizeof(a2);
    sizeof(a3);
}

foo2_t a2[10];

void g(void)
{
    a2 = a2[0];
    sizeof(a2);
}

void h(void)
{
    foo1_t a1[10];
    foo2_t a2[];
    foo3_t a3[10][];

    a1 = a1[0];
    a2 = a2[0];
    a3 = a3[0];

    sizeof(a1);
    sizeof(a2);
    sizeof(a3);
}

void q(void)
{
    foo1_t *a1[10];
    foo2_t *a2[];
    foo3_t *a3[10][];

    a1 = a1[0];
    a2 = a2[0];
    a3 = a3[0];
}
