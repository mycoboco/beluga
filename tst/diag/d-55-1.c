void f(void)
{
    int a;

    (foo1_t) a;
    (foo2_t ()) a;
    (foo3_t *) a;
    (foo4_t *a) a;
    (foo5_t const) a;
    (foo6_t volatile *) a;
    (foo7_t []) a;
    ((foo8_t)()) a;
}
