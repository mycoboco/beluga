void f1(void) { int a[10]; int *p; p = *&a; }
void f2(void) { void (*p)(void); p = *&f2; }
void f3(void) { int *p; int i; i = *p; }
void f4(void) { int i; i = *i; }
