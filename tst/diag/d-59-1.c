static struct tag {
    foo1_t b1: -1;
    foo2_t b2: 99;
    foo3_t b3: 0;
    foo4_t *x;
    foo5_t f(a);
    const foo6_t m;
    int;
    foo7_t;
} w;

struct tag w2, w3 = w2;

void f1(void)
{
    w2.x + w2;
    w3 = w2;
    *w2;
}

struct tag2 {
    int m1;
    struct {
        foo8_t *m2;
    } m3;
    const struct {
        foo9_t *m3;
    };
};

void f2(void)
{
    struct tag2 q, r;

    q = r;
    q.m3 = r.m1;
}

void f3(void)
{
    union tag3 {
        int m1;
        union {
            foo10_t *m2;
        } m3;
        volatile union {
            const foo11_t m3[2];
        };
    } q, r = q;

    q.m3 = r.m1;
}
