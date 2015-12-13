void f1(void) { double x; char c; x = c * x; }
void f2(void) { int *p; char c; p = c * p; p = p * p; }    /* error */
void f3(void) { struct t x; x = x * x; }    /* error */
