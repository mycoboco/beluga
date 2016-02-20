void f1(void) { struct tag { int x; } x; x.x = (x)? x.x: x.x; }
void f2(void) { int *x; double *y; void *p; p = (p)? x: y; }
void f3(void) { int x; x = (3.14)? x: x+1; x = (3.14F)? x: x+1; x = (3.14L)? x: x+1; }
void f4(void) { int x; x = (2+0xFF)? x: x+1; x = (2U)? x: x+1; x = (0xFFFFFFFFUL+1)? x: x+1; }
void f5(void) { int *x; x = ((char *)0)? x: x+1; x = ((char *)0xFF)? x: x+1; }
void f6(void) { void f(void), g(void); int x; x = (x)? f(): g(); x = (x)? (void)f: (void)g; }
