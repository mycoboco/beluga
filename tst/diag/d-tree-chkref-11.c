int f()
{
    int g();
    int y, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10;

    y = sizeof(x1 + 3.14);
    y = (x2 + 3.14);                           /* warning */

    while (x3);                                /* warning */
    for (f(&x4); (&x4 + (&x1-&x2))[y]; );
    for (; (&x5 + (&x1-&x2))[y]; );            /* warning */
    if (x6, x7);                               /* warning */

    switch(*(&x8 + g(&x8))) { case 0:; }
    switch(*(&x9 + (&x1-&x2))) { case 0:; }    /* warning */

    return y[&x10];                            /* warning */
}
