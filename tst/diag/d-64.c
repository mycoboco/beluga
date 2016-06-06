struct {
    int;
    bar_t;
};

void f1(*p);
void f2(*p) {}
void f3(*p, *q);
void f4(*p, *q) {}
void f5(*p, int);
void f6(*p, int) {}

void g1(foo1_t, int);
void g2(foo2_t, int) {}

void h1(foo) foo3_t; {}
void h2(foo) foo4_t; foo5_t; {}
void h3(foo) foo6_t [10]; {}

void i1(int p, foo7_t, foo8_t *);
void i2(int p, foo9_t, foo10_t [10]);

void j1(+[10]);
void j2(++foo11_t);
