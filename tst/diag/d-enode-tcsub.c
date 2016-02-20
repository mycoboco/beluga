void f1(void) { char c; double x; c = x - c; }
void f2(void) { int *p; char c; p = f2 - c; p = c - f2; }
void f3(void) { int *p; char c; p = p - c; }
void f4(void) { int *p; char c; c = p - p; }
void f5(void) { void *p, *q; char c; c = p - q; }
void f6(void) { void *p; char c; c = p - 0; }
void f7(void) { int (*p)[]; int (*q)[10]; char c; c = p - q; }
void f8(void) { int (*p)[10]; int (*q)[10]; char c; c = p - q; }
void f9(void) { int (*p)[10]; int (*q)[11]; char c; c = p - q; }
void f10(void) { int * const p; int *q; char c; c = p - q; }
void f11(void) { struct t x; char c; c = x - x; x = x - c; }
