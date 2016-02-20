void f1(void) { static int x = 0 = 1; }
void f2(void) { extern int x = 0; }
void f3(void) { extern void f2(void) = 1; }
void f4(void) { extern void f3(void) = { 0, 1, 2 }; }
void f5(void) { typedef int int_t = 12; }
void f6(void) { int x = { 0, }; }
void f7(void) { int x = { 1 }; }
void f8(void) { int y = 0, x = { y }; }
void f9(void) { int x = x; }
struct tag { double x; }; void f10(void) { struct tag x = 0.0; }
void f11(void) { struct tag x = { 0.0 }, y = x, z = { y }; }
void f12(void) { int x[] = { 0, 1, 2, }; sizeof(x); }
void f13(void) { int x[][2] = { 0, 1, 2, }; sizeof(x); }
void f14(void) { int x[]; }
