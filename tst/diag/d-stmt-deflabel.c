void f(void)
{
    f();
    goto lab;
    lab:
        ;
    goto lab;
}

void g(void)
{
    f();
    goto lab3;
    lab1: {}
    lab2: {}
    lab3: {}
    goto lab2;
}
