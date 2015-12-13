void f(void)
{
    int i;
    int *p;
    unsigned u;
    double d;

    +(i = 0);
    +(u = 0);
    +(d = 0);

    (i = 0) + 0;
    (p = 0) + 0;
    (u = 0) + 0;
    (d = 0) + 0;

    (i = 0) - 0;
    (p = 0) - 0;
    (u = 0) - 0;
    (d = 0) - 0;

    (i = 0) * 0;
    (u = 0) * 0;
    (d = 0) * 0;

    (i = 0) * 1;
    (u = 0) * 1;
    (d = 0) * 1;

    (i = 0) / 1;
    (u = 0) / 1;
    (d = 0) / 1;
}

void g(void)
{
    int x;
    unsigned u;
    double d;

    x = 0;
    (void)(x = 0);
    (int)(x = 0);         /* warning */
    (unsigned)(x = 0);    /* warning */
    (double)(x = 0);      /* warning */

    u = 0;
    (void)(u = 0);
    (int)(u = 0);         /* warning */
    (unsigned)(u = 0);    /* warning */
    (double)(u = 0);      /* warning */

    d = 0;
    (void)(d = 0);
    (int)(d = 0);         /* warning */
    (unsigned)(d = 0);    /* warning */
    (double)(d = 0);      /* warning */
}
