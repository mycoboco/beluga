void f1(void) { int a; unsigned b; a = a | b; }
void f2(void) { int *p; int c; c = p ^ p; }    /* error */
void f3(void) { struct t x; int c; c = x & x; }    /* error */
