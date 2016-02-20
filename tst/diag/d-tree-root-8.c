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
    (int)(x = 0);
    (unsigned)(x = 0);
    (double)(x = 0);

    u = 0;
    (void)(u = 0);
    (int)(u = 0);
    (unsigned)(u = 0);
    (double)(u = 0);

    d = 0;
    (void)(d = 0);
    (int)(d = 0);
    (unsigned)(d = 0);
    (double)(d = 0);
}
