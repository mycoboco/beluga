int *f(int x)
{
    if (f(x))
        return &x;
    else if (!f(x))
        return 1 + &x;
    else if (1)
        return &((&x)[1]);
    else
        return (int *)((unsigned)&x ^ (unsigned)&x);
    f(x);
    return 0;
}

void g(void)
{
    int x;
    if (f(x))
        return &x;
    else if (!f(x))
        return 1 + &x;
    else
        return &((&x)[1]);    /* warniing */
}
