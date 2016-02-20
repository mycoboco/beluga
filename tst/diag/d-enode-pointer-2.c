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

    &z;
    z;
    &g();

    &z.x;
    z.x;
    w.x;
    &g().x;

    &z.y;
    z.y;
    w.y;
    &g().y;

    &z.z.x;
    z.z.x;
    w.w.x;
    &g().z.x;

    sizeof(x);
    sizeof(x+0);

    x+0;
    x;

    for (x;
         x;
         x);
    for (x+0; sizeof(x+0);
         sizeof(x));

    while (x);
    while (x,
           x);

    1, x;
    x, 1;

    y = 0, x;
    x, y = 0;

    0 && x;
    1 && x;
    0 || x;
    1 || x;

    switch(x) {}

    return x;
    return y;
    return z.x;
    return z.y;
    return w.x;
    return w.y;
    return z.z.x;
    return z.z.w;
    return w.w.x;
    return w.w.y;
    return sizeof(z.x);
    return sizeof(z.z.x);
}
