void f(void)
{
    sizeof(foo1_t);
    sizeof(foo2_t ());
    sizeof(foo3_t *);
    sizeof(foo4_t *a);
    sizeof(foo5_t const);
    sizeof(foo6_t volatile *);
    sizeof(foo7_t []);
    sizeof((foo8_t)());
}
