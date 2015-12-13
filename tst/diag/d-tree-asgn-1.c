void f1(void) { struct tag x, y; x = y; }    /* error */
void f2(void) { double x; int y; x = 3; y = x; }
void f3(void) { struct tag *x; x = 3.14; x = (void)3.14; }    /* error */
void f4(void) { void x; x = 0; }    /* error */
void f5(void) { const int x, *px; x = 0; *px = 1; }                 /* error */
void f6(void) { struct { const int x; } x, y; x = y; }              /* error */
void f7(void) { struct tag { const int x; } *f(), x; *f() = x; }    /* error */
void f8(void) { struct { int z; int :2, x:10; } x; int y; x.x = y; }
void f9(void) { struct { int z; int :2, x:10; } x; x.x = 7; }
void f10(void) { struct { int z; unsigned int :2, x:10; } x; int y; x.x = 7; x.x = y; }
void f11(void) { struct { int z; char :2, x:4; } x; x.x = 7; }    /* error */
void f12(void) { struct { struct { const int x; } x; } *g(), x; *g() = x; }    /* error */
void f13(void) { union { union { const int x; int y; } x; } *h(), x; *h() = x; }    /* error */
void f14(void) { struct { const int x[10]; } *q(), x; *q() = x; }    /* error */
void f15(void) { union { const int x[10]; int y; } *w(), x; *w() = x; }    /* error */
