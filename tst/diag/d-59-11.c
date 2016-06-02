foo1_t foo1_t;
bar1_t *foo1_t;

struct {
    foo2_t foo2_t;
    foo2_t foo2_t;
};

int foo1_t[];

void f1(foo3_t foo3_t);
void f2(foo4_t foo4_t) { foo4_t[]; }

void f3(void)
{
    foo5_t *foo5_t;
    extern foo6_t foo6_t();
}

enum {
    foo1_t,
    bar1_t
};

typedef foo7_t *foo7_t;
foo7_t foo7_t;

foo8_t x8;
void f4(int foo8_t);
void f5(int foo8_t) { foo8_t++; }

void f6(foo9_t x) { int *foo9_t; foo9_t << 0; }

void f7(void)
{
    foo10_t x10;
    {
        double foo10_t;
        foo10_t << 0;
    }
    {
        extern int foo10_t;
        *foo10_t;
    }
}

void f8(a, b)
foo11_t foo11_t;
{
    foo11_t++;
    {
        double *foo11_t;
        *foo11_t;
    }
}
