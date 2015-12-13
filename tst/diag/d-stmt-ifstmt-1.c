double f1(void)
{
    extern int f(void);

    if (f())
        f1();
    else if (f1() != 0)
        f1();
    else
        return 0;

    if (f() + 1) {
        if (f1() + 1)
            return f1();
    } else
        return 3.14;

    return 0;
}
