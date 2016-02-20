extern void q, *pv;

int f(void)
{
    int x, y;

    y = 0;
    y + 0;
    y * 0;
    !!x + !!(x = 0);
    !!x | !!(x, x = 0);
    ~!!x;
    x = ~~(x = 0) + !!-(x++) + !!-(++x);
    (*(x, &x))++;
    (void)(x, f());
    (x, x=0);
    !!!!x + ((x, x=0), x=0);
    x, x = 0, x = 1;
    (void)x;
    (char)x;
    (unsigned)x;
    (double)x;
    (int *)x;
    *(int *)x;
    (int)x;
    q;
    (void)q;
    (const void)q;
    *pv;
    (void)*pv;
    (const void)*pv;
}
