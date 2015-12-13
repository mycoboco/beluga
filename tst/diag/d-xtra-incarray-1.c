/* --std=c90 -Wv */

extern void x[10];
extern void y[10][10];
extern void z[10][];
extern void i;

void f(void)
{
    void *q;

    q = x;
    q = &x;
    q = &*x;
    *x = 0;
    x[0] = 0;
    x[1] = 0;
    i = x[0];
    i = x[1];

    q = y;
    q = *y;
    q = &y;
    q = &*y;
    q = &**y;
    *y = 0;
    **y = 0;
    y[0][0] = 0;
    y[1][1] = 0;
    i = y[0][0];
    i = y[1][1];

    q = z;
    q = *z;
    q = &z;
    q = &*z;
    q = &**z;
    *z = 0;
    **z = 0;
    z[0][0] = 0;
    z[1][1] = 0;
    i = z[0][0];
    i = z[1][1];
}

extern const void X[10];
extern volatile void Y[10][10];
extern const void Z[10][];

void g(void)
{
    void *q;

    q = X;
    q = &X;
    q = &*X;
    *X = 0;
    X[0] = 0;
    X[1] = 0;
    i = X[0];
    i = X[1];

    q = Y;
    q = &Y;
    q = *Y;
    q = &*Y;
    q = &**Y;
    *Y = 0;
    **Y = 0;
    Y[0][0] = 0;
    Y[1][1] = 0;
    i = Y[0][0];
    i = Y[1][1];

    q = Z;
    q = &Z;
    q = *Z;
    q = &*Z;
    q = &**Z;
    *Z = 0;
    **Z = 0;
    Z[0][0] = 0;
    Z[1][1] = 0;
    i = Z[0][0];
    i = Z[1][1];
}

void h(void)
{
    struct tag x[10];
    struct tag y[10][10];
    struct tag z[10][];
    struct tag i;

    void *q;
    q = x;
    q = &x;
    q = &*x;
    *x = i;
    x[0] = i;
    x[1] = i;
    i = x[0];
    i = x[1];

    q = y;
    q = &y;
    q = *y;
    q = &*y;
    q = &**y;
    *y = i;
    **y = i;
    y[0][0] = i;
    y[1][1] = i;
    i = y[0][0];
    i = y[1][1];

    q = z;
    q = &z;
    q = *z;
    q = &*z;
    q = &**z;
    *z = i;
    **z = i;
    z[0][0] = i;
    z[1][1] = i;
    i = z[0][0];
    i = z[1][1];
}
