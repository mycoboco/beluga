void f1(void) { struct { int x; } x; x = x + x; }
void f2(void) { long double x; char i; x = x + i; }
void f3(void) { int *p; p = p + p; p = p + (void *)p; }
void f4(void) { int *p; double x; p = p + x; }
void f5(void) { int *p; unsigned u; p = p + u; p = u + p; }
void f6(void) { int *p; int i; i = p + i; }
void f7(void) { struct tag *p; p = p + 0; }
void f8(void) { void *p; p = 1 + *&p; }
