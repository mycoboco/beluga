void f1(void) { char c; double x; int i; i = c + x; }
void f2(void) { int *p; char c; p = p + c; p = c + p; }
void f3(void) { int *p; char c; p = f3 + c; p = c + f3; }    /* error */
void f4(void) { struct t x; char c; c = x + c; }    /* error */
