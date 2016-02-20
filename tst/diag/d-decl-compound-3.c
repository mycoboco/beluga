/* --std=c90 -Wv */

void f(void)
{
    return;
    int a;
    a = 0;
}

void g(void)
{
    goto lab;
    extern int a;
    goto lab;
    lab:
        ;
}

void h(void)
{
    typedef int foo;
    return;
    foo a = 0;
    a++;
}
