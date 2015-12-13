void f1(void) { int a[10], *p; p = a; }
void f2(void) { void (*pf)(void); pf = f2; }
