void f(void)
{
    if (0)
        return 1;
}

double f2(void)
{
    return;
    goto 0;
    lab:
        goto lab;
    goto el;
        el:
        ;
    short int a;
    return 3.14;
}

void f3(void)
{
    {
        label:
    }
    f();
}
