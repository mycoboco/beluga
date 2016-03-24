void f(void)
{
    int a, b, c, d();

    if (&a && a+b || *c)    /* error */
        ;

    if (&a && a+b || d())    /* no paren */
        ;

    if ((a = b && c) && b || c)    /* no paren */
        ;

    if (1 || a && b)    /* no paren */
        ;
    if (0 && a || b)    /* no paren */
        ;

    if (a && b, c || d)
        ;
    if ((a, b && c) || d)
        ;
    if (a && b || c, c || b && a)    /* no paren x 2 */
        ;
}

void g(void)
{
    int a, b, c;
    int t = 1 || 2 && 3;    /* no paren */

    t = a && b || c;    /* no paren */
}

static int q = &f && &g || &f;    /* no paren */
static int a[] = { &f && &g || &f,    /* no paren */
                   1 || 2 && 3 };     /* no paren */
static int b[1 && 2 || 3];    /* no paren */
