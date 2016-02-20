void f1(void) { struct { int x; } x; x = x - x; }
void f2(void) { double x; char c; c = x - c; }
void f3(void) { struct tag *x; x = x - x; x = x - 0; }
void f4(void) { int *x; unsigned u; x = x - 1; x = x - u; }
