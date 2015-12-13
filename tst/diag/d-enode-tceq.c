void f1(void) { int *p; int i; i = 0 == p; i = (void *)0 == p; }
void f2(void) { void *p; int *q; int i; i = p == q; i = p == f2; }    /* error */
void f3(void) { int (*p)[]; int (*q)[10]; int i; i = p == q; }
void f4(void) { int (*p)[10]; int (*q)[20]; int i; i = p == q; }    /* error */
void f5(void) { char c; double x; int i; i = c == x; }
void f6(void) { struct t x, y; int i; i = x == y; }    /* error */
void f7(void) { struct t { int x; } x, y; int i; i = x == y; }    /* error */
void f8(void) { int * const p; int *q; int i; i = p != q; }
void f9(void) { void *p; int i; i = p == p; }
