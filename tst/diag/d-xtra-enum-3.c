typedef enum e1 { A } e1;
typedef enum e2 { B } e2;

void func(void)
{
    float f;
    int *pi;
    e1 x1, *px1;
    e2 x2, *px2;
    struct { int m; } str;

    x1 = x2;
    px1 = px2;
    px1 = pi;
    pi = px1;

    x1 = str;
    (x1) = str;
    x1[x2];
    (x1[pi]) = str;
    (px1[x1]) = str;
    x1.m;
    x1->m;
    x1++;
    (x1++) = str;
    px1++;
    x1--;
    (x1--) = str;
    px1--;
    x1();

    ++x1;
    (++x1) = str;
    --x1;
    (--x1) = str;
    &x1;
    px1 = &x1;
    px2 = &x1;
    pi = &x1;
    (&x1) = str;
    *x1;
    (*px1) = str;
    x1 = *px1;
    +x1;
    (+x1) = str;
    -x1;
    (-x1) = str;
    ~x1;
    (~x1) = str;
    !x1;
    (!x1) = str;
    sizeof(x1);

    (x1 * x1) = str;
    (x1 / x1) = str;
    (x1 % x1) = str;
    (x1 * f) = str;

    (x1 + x1) = str;
    (x1 + x2) = str;
    (x1 + f) = str;
    (px1 + x1) = str;

    (x1 << 1) = str;
    (1 << x1) = str;
    (1u << x1) = str;
    (x1 >> 1) = str;
    (1 >> x1) = str;
    (1u >> x1) = str;

    (x1 < x2) = str;
    (x1 < 1) = str;
    (x1 < f) = str;

    (x1 == x2) = str;
    (x1 != f) = str;
    x1 == pi;

    (x1 & x2) = str;
    (x1 & 1u) = str;
    (x1 & 0) = str;

    (x1 && x2) = str;
    (x1 || f) = str;
    (x1 && p1) = str;

    (1? x1: x2) = str;
    (1? x1: 1) = str;
    (1? x1: x1) = str;
    (1? x1: pi) = str;

    (x1 = x1) = str;
}
