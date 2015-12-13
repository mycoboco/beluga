int *f(void)
{
    int y;
    register int x[10];
    register struct x {
        int w;
        int x[10], y;
        struct {
            int w;
            int x[10];
        } z;
    } z;
    register struct y {
        int x[10];
        char y;
        struct {
            int x[10];
            int y;
        } w;
    } w;
    extern struct x g();

    &z;      /* error */
    z;
    &g();    /* error */

    &z.x;      /* error */
    z.x;       /* error */
    w.x;       /* error */
    &g().x;    /* error */

    &z.y;      /* error */
    z.y;
    w.y;
    &g().y;    /* error */

    &z.z.x;      /* error */
    z.z.x;       /* error */
    w.w.x;       /* error */
    &g().z.x;    /* error */

    sizeof(x);
    sizeof(x+0);    /* error */

    x+0;    /* error */
    x;      /* error */

    for (x;                   /* error */
         x;                   /* error */
         x);                  /* error */
    for (x+0; sizeof(x+0);    /* error */
         sizeof(x));

    while (x);    /* error */
    while (x,     /* error */
           x);    /* error */

    1, x;    /* error */
    x, 1;    /* error */

    y = 0, x;    /* error */
    x, y = 0;    /* error */

    0 && x;    /* error */
    1 && x;    /* error */
    0 || x;    /* error */
    1 || x;    /* error */

    switch(x) {}    /* error */

    return x;                /* error */
    return y;
    return z.x;              /* error */
    return z.y;
    return w.x;              /* error */
    return w.y;
    return z.z.x;            /* error */
    return z.z.w;
    return w.w.x;            /* error */
    return w.w.y;
    return sizeof(z.x);
    return sizeof(z.z.x);
}
