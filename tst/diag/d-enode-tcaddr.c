void f1(void) { int (*p)[]; int a[10]; p = &a; }
void f2(void) { void (*p)(void); p = &f2; }
void f3(void) { int x, *p; p = &x; }
void f4(void) { int x, *p; p = &+x; }    /* error */
void f5(void) { int *p; int i; p = &((i)? i: i); }    /* error */
void f6(void) { int *p; int i; p = &((int)i); }    /* error */
void f7(void) { struct t f(), *p; p = &(f()); }    /* error */
void f8(void) { struct t { int x; } g(); int *p; p = &(g().x); }    /* error */
