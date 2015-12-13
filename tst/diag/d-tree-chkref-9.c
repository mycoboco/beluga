void f(void)
{
    int y, *p, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10;

    p = (void *)(unsigned long)&x1;
    *p = 0;
    y = x1;

    y = *(p = &x2);
    y = x2;

    y = (x3 && x4);    /* warning */
    y = (1 && x5);     /* warning */
    y = (0 && x6);

    y = (x7 || x8);    /* warning */
    y = (1 || x9);
    y = (0 || x10);    /* warning */
}
