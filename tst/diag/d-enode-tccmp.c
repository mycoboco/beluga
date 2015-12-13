void f1(void) { int i; double d; i = i < d; }
void f2(void) { void *p; int *q; int i; i = p > q; }    /* error */
void f3(void) { void *p; int i; i = p < 0; }    /* error */
void f4(void) { void *p; int i; i = p >= (void *)0; }
void f5(void) { void *p; int i; i = p <= p; }
void f6(void) { int *p; int i; i = p > p; }
void f7(void) { int (*p)[]; int (*q)[10]; int i; i = p > q; }    /* error */
void f8(void) { int (*p)[10]; int (*q)[10]; int i; i = p < q; }
void f9(void) { int (*p)[10]; int (*q)[11]; int i; i = p < q; }    /* error */
void f10(void) { int i; i = f10 > f10; }    /* error */
void f11(void) { int * const p; int *q; int i; i = p < q; }
void f12(void) { struct t x; int i; i = x > x; }    /* error */
