void f1(int a = 0, void *x() = { 1, });
void f2(int = { 0, }, void *() = 1);
void f3(void = 2);
void f4(void, void x = 3);
void f5(void, void = { 5, });
void f6(const void, const void = 6);
void f7(int, void, void z = 7);
void f8(int, void = 8);
void f9(int, ..., int x() = 0);
void f10(int, ..., int * = { i, });
void f11(void, int x = 0);
void f12(void, int x = 0, int y = 0);
void f13(void, int = { j });
void f14(int, a);
void f15(int, a, float);
void f16(int, double) int a; {}
void f17(int) int a = { { 0, } }; {}

void g1(a = 0);
void g2(a = 0, b = 0);
void g3(a = 0, b, c = 0);
void g4(a, b = { 1+1 });
void g5(a, = 0);
void g6(a, = { 1+1 });
void g7(= 0);
void g8(= { 0, });
void g9(= 0, b);
void g10(a, int x = 0, int *y = 0);
void g11(a, ..., int x);
void g12(a, ...);
void g13(a, b) int a = 0, b = { { 0 } }; {}
