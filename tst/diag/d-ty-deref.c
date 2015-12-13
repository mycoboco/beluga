const enum { A = 0 } *pe, e;
void f1(void) { int x; *x; x[0]; }
void f2(void) { int x; *pe = x; pe = &x; e = *pe + 0; *pe = e + 0; }
