void f1(void) { union { int a; } x; x.int; x->double; }
void f2(void) { int x; x.a; x->a; }
void f3(void) { struct tag { int x; } f(void); int y; y = f().x; f().x = 0; }
void f4(void) { struct { struct { int x; } x; } x; int y; y = x.x.x; }
void f5(void) { union { union { int x; } *x; } *x; int y; y = x->x->x; }
void f6(void) { struct { int a; } x; x.b; (&x)->b; }
void f7(void) { const volatile struct { int a[10]; } x; x.a[0] = 0; x.a; x.a[0]; }
void f8(void) { struct { int a[]; } x; x.a[0] = 0; x.a; x.a[0]; }
void f9(void) { const volatile struct { int a; } x; x.a = 0; x.a; }
void f10(void) { const struct { struct tag x; } x; x.x = x.x; x.x; }
void f11(void) { struct { long double x; int y; } x; x.y = x.y; }
void f12(void) { struct { long double x; int y; } *x; x->x = x->y; }
