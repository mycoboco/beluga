void f1(void)   { volatile int x; struct tag y; x+2; y; }    /* ERROR, warning */
void f2_3(void) { volatile int x[10]; struct tag *y; x;
                                                   *(x+1); 10[x]; *(y); }    /* warning */
void f4(void)   { struct { volatile struct { int x; } x; } x; x; x.x.x; }    /* warning */
void f4_8(void) { volatile struct { int a[2][2]; } x; x;           /* warning */
                                                      x.a;
                                                      x.a[0];
                                                      *x.a[0], x.a[0][0]; }    /* warning */
void f9(void) { struct { struct { volatile int x; } x; } x; x; }    /* warning */
void f10(void) { const struct { volatile int x; } *x; *x; }    /* warning */
void f11(void) { struct { volatile int x; } * const x; *x; }    /* warning */
void f12(void) { struct { volatile int x; } * const * const x; **x; }    /* warning */
void f13(void) { struct { volatile int x[10]; } x; x; }    /* warning */
void f14(void) { const struct { volatile int x[10]; } *x; *x; }    /* warning */
void f15(void) { struct { volatile int x[10]; } * const x; *x; }    /* warning */
void f16(void) { struct { volatile int x[10]; } * const * const x; **x; }    /* warning */
