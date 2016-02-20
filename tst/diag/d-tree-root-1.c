void f1(void)   { volatile int x; struct tag y; x+2; y; }
void f2_3(void) { volatile int x[10]; struct tag *y; x;
                                                   *(x+1); 10[x]; *(y); }
void f4(void)   { struct { volatile struct { int x; } x; } x; x; x.x.x; }
void f4_8(void) { volatile struct { int a[2][2]; } x; x;
                                                      x.a;
                                                      x.a[0];
                                                      *x.a[0], x.a[0][0]; }
void f9(void) { struct { struct { volatile int x; } x; } x; x; }
void f10(void) { const struct { volatile int x; } *x; *x; }
void f11(void) { struct { volatile int x; } * const x; *x; }
void f12(void) { struct { volatile int x; } * const * const x; **x; }
void f13(void) { struct { volatile int x[10]; } x; x; }
void f14(void) { const struct { volatile int x[10]; } *x; *x; }
void f15(void) { struct { volatile int x[10]; } * const x; *x; }
void f16(void) { struct { volatile int x[10]; } * const * const x; **x; }
