void f1(void) { int p; p = *p; }
void f2(void) { void f(), (*pf)(); pf = *f; pf = **f; pf = ******f; }
void f3(void) { int a[10], *p, i; p = a; i = *a; }
void f4(void) { int *p, i; i = *p; }
