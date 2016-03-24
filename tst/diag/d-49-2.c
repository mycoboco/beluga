void f(void)
{
    int a, b, c, d;

    if (a || b && c)    /* no paren */
        ;
    if (a && b || c)    /* no paren */
        ;

    if (a || (b && c))
        ;
    if (a && (b || c))
        ;

    if (a && b && c || d)    /* no paren */
        ;
    if (a || b && c && d)    /* no paren */
        ;

    if (a && (b && c) || d)    /* no paren */
        ;
    if (a || b && (c && d))    /* no paren */
        ;

    if ((a && (b && c)) || d)
        ;
    if (a || (b && (c && d)))
        ;
}
