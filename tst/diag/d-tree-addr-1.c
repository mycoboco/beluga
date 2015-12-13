void f1(void) { int a, f(void); &+a; &f(); }    /* error */
void f2(void) { int a[10], (*p)[]; p = &a; }
void f3(void) { int f(void), (*pf)(void); pf = &f; }
void f4(void) { void *p; int i; p = &i; }
