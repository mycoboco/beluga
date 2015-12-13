struct tag { int x; };
void f1(struct tag a) { struct tag x; x = a; }
void f2(void) { int a[10], b[]; int x; x = a[0]; x = b[1]; }    /* error */
void f3(void) { struct t a[10]; struct t x; x = a[1]; }    /* error */
void f4(void) { int h4(void); int (*p)(void); p = h4; }
void f5(void) { int i; double d; int *pi; pi = pi; i = i; d = d; }
