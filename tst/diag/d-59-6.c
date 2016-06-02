typedef foo1_t bar1_t;
typedef foo2_t *bar2_t;
typedef foo3_t bar3_t(void);
typedef foo4_t bar4_t[];
typedef struct { foo5_t m; } bar5_t;

bar1_t x;

void f(void)
{
    bar2_t x2;
    bar4_t x4;
    bar5_t x5;

    x++;
    x = x2;
    x4++;
    x5.m = x;

    {
        bar2_t x2;
        bar6_t x6;

        x2 = *x2;
        *x6[0] = 0;
    }
}

bar3_t g {}
