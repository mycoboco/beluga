void f(void)
{
    int a;

    if (a = 0)    /* no paren */
        ;
    if ((a = 0))
        ;
    if (a = a)    /* no paren */
        ;
    if (a = a, a = a)
        ;
    if ((a = 0) == 0)
        ;
    if (1 = 0)
        ;
    if ((a = 0)+1)
        ;
    if ((a = 0)+0)
        ;
    if (!(a = 0))
        ;
    if (!!(a = 0))
        ;
    if (a += 0)
        ;
    if (a++)
        ;
    if (++a)
        ;
    if (1? a = 0: 1)
        ;
    if (1? (a = 0): 1)
        ;
    if (a? a = 0: 1)
        ;
    if (a = 1? 0: 1)    /* no paren */
        ;
    if ((a = 1? 0: 1))
        ;
    if (a = (1? 0: 1))    /* no paren */
        ;
    if (a = a? 0: 1)    /* no paren */
        ;
    if ((a = a? 0: 1))
        ;
    if (a = (1? 0: 1))    /* no paren */
        ;
    if ((a = 0) | 0)
        ;
    if ((a = 0) << 0)
        ;
    if ((a = 0) * 0)
        ;
    if (a = a && 1)    /* no paren */
        ;
    if (a = 0 && a)    /* no paren */
        ;
    if (-(-(a = a)))
        ;

    for (a = 0; a = 0; a = 0)    /* no paren */
        ;
    while (a = a)    /* no paren */
        ;
    do; while (a = a);    /* no paren */
}
