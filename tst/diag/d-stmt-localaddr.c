int *f(int x)
{
    if (f(x))
        return &x;    /* warning */
    else if (!f(x))
        return 1 + &x;    /* warning */
    else if (1)
        return &((&x)[1]);    /* warning */
    else
        return (int *)((unsigned)&x ^ (unsigned)&x);    /* warning */
    f(x);    /* warning */
    return 0;
}

void g(void)
{
    int x;
    if (f(x))
        return &x;    /* warning */
    else if (!f(x))
        return 1 + &x;    /* warning */
    else
        return &((&x)[1]);    /* warniing */
}
