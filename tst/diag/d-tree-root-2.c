extern void q, *pv;

int f(void)
{
    int x, y;

    y = 0;
    y + 0;                                  /* warning */
    y * 0;                                  /* warning */
    !!x + !!(x = 0);                        /* warning */
    !!x | !!(x, x = 0);                     /* warning */
    ~!!x;                                   /* warning */
    x = ~~(x = 0) + !!-(x++) + !!-(++x);
    (*(x, &x))++;                           /* warning */
    (void)(x, f());                         /* warning */
    (x, x=0);                               /* warning */
    !!!!x + ((x, x=0), x=0);                /* warning */
    x, x = 0, x = 1;                        /* warning */
    (void)x;
    (char)x;                                /* warning */
    (unsigned)x;                            /* warning */
    (double)x;                              /* warning */
    (int *)x;                               /* warning */
    *(int *)x;                              /* warning */
    (int)x;                                 /* warning */
    q;
    (void)q;
    (const void)q;
    *pv;
    (void)*pv;
    (const void)*pv;
}
