void f1(foo1_t a[]) { a[0] = a; }
void f2(foo2_t a[10][]) { a[0] = a; }
void f3(foo3_t f()) { f() = 0; }
void f4(void f()) { f() = 0; }
void f5(void f(foo5_t *)) { foo5_t g; f() = 0; g = 0; }
