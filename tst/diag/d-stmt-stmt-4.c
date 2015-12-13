void f(void)
{
    if (0)
        return 1;    /* error */
}

double f2(void)
{
    return;    /* warning */
    goto 0;    /* error */
    lab:
        goto lab;
    goto el;
        el:
        ;
    short int a;    /* error */
    return 3.14;
}

void f3(void)
{
    {
        label:
    }
    f();
}
