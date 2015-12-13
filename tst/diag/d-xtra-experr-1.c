/* -Wv --_verbose-experr */

void f(void)
{
    short x5, *p;
    typedef double x6;
    float x7;
    struct { int x; } s;
    struct tag *ps;

    (x11() + f) * f;
    (f + f) * x12();

    (x14 + f) / f;
    (f + f) / x15;

    (*x6 + f) % f;
    (f + f) % *x6;

    (x5 + []) - f;
    (x5 + f) - [];

    (f++)++;
    (f + f)++;
    (f--)--;
    (f / f)--;

    x5[x5][x5];
    (f + f)[x5];

    x5() - p;
    (x5 - p)();

    x5.m + f;
    (x5 + f).m;

    x5->m - f;
    (x5 | f)->m;

    ~*x7;
    *~x7;

    (&1)();
    &~x7;

    (+f)++;
    +(++f);

    (-f)--;
    -(--f);

    (!s).m;
    !(s + s);

    ++(++f);
    --(--f);

    sizeof(f) + f;
    sizeof(*ps + *ps);

    ((int [10])0)[x7];
    (int [10])(p[x7]);

    (f ^ f) + 1;
    (f + 1) ^ f;

    (f / f) + 1;
    (f + 1) / f;

    (s && f) || s;
    (f + 1) || s;

    (f < 0) << f;
    (f << 0) < 0;

    (s = 0) > 0;
    *(s > 0) = f;
    (*ps = *ps)[0];
    *(ps+ps) = *ps;

    ((f)? s: f) + f;
    ((f)? f + f: s) + f;
    ((s)? s: f) + f;

    f+f, *s, s+f;
}
