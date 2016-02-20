void f1(void) { int *p; p = 0; p = 1; }
void f2(void) { int *p; p = 1-2+1; p = (1, 0); }
void f3(void) { int *p; p = (void *)0; }
void f4(void) { int *p; p = (int *)1 - (int *)1; }
void f5(void) { int *p; p = (int *)0; }
void f6(void) { int *p; p = (double *)0; }
void f7(void) { int (*p)(void); p = 0; p = 1; }
void f8(void) { int (*p)(void); p = 1-2+1; p = (1, 0); }
void f9(void) { int (*p)(void); p = (void *)0; }
void f10(void) { int (*p)(void); p = (int *)1 - (int *)1; }
void f11(void) { int (*p)(void); p = (int (*)(void))0; }
void f12(void) { int (*p)(void); p = (double *)0; }
