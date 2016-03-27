int a;

void f(void)
{
    if (a & 1)
        ;
    if (a | 1)
        ;
    if (a ^ 1)
        ;

    if (a & 1 < 0)     /* no paren */
        ;
    if (a | 1 == 0)    /* no paren */
        ;
    if (a ^ 1 > 0)     /* no paren */
        ;
    if (a & a <= 0)    /* no paren */
        ;
    if (a | a >= 0)    /* no paren */
        ;
    if (a ^ a != 0)    /* no paren */
        ;

    if (0 <= a & a)    /* no paren */
        ;
    if (0 == a | a)    /* no paren */
        ;
    if (0 != a ^ a)    /* no paren */
        ;

    if (0 <= a & a < 0)    /* no paren x 2 */
        ;

    if ((a & 1) < 0)
        ;
    if (a | (1 == 0))
        ;
    if (a ^ (1 > 0))
        ;

    if ((0 <= a) & a)
        ;
    if (0 != (a ^ a))
        ;

    if ((0 <= a) & a < 0)    /* no paren */
        ;
    if ((0 <= a) & (a < 0))
        ;

    if (1 & 0 != 0)    /* no paren */
        ;
    if (1 | 1 == 1)    /* no paren */
        ;
    if (a & 0 == 1)    /* no paren */
        ;

    if (0 != 1 & 0)    /* no paren */
        ;
    if (1 == 1 | 1)    /* no paren */
        ;

    if ((0 == a) & 0 == 1)    /* no paren */
        ;
    if (0 == (a & 0) == 1)
        ;
    if (0 == a & 0 == 1)    /* no paren x 2 */
        ;

    if (a & 1 && a | 1)
        ;
    if (a & 1 && a | 1 || a ^ 1)    /* no paren for && */
        ;
}

void g(void)
{
    unsigned u;
    int a, b;
    struct { int m:1, n:2; } x;

    if ((a && b) | 1)
        ;

    if (x.n == 0 & 1)    /* no paren */
        ;
    if ((x.n == 0) & 1)
        ;
    if (x.n == (0 & 1))
        ;

    if (u <= 0 & 1)    /* no paren */
        ;
    if (u > 0 & 1)     /* no paren */
        ;
}
