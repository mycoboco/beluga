foo1_t f1(void);
void f2(foo2_t q);
void f3(void, foo3_t *);
void f4(int, foo4_t);
void f5(foo5_t, int);

void f(void)
{
    int x = f1();
    f1() = f2();
    x = f3();
    f4() = 0;
    f5() = f5();
}

void f6(foo6_t x, foo7_t) { x = foo7_t; }
void f7(int x, foo7_t y) { x = y; }
void f8(int x, foo8_t, foo9_t) { foo8_t = x = foo9_t; }
foo10_t f10(foo11_t *p, ..., foo12_t) { p = f10(); }
