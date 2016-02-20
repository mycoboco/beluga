void f1(auto int a, b);
void f2(int a, b);
void f3(register int a, b);
void g1(a, b) auto int a, b; { }
void g2(a, b) int a, b; { }
void g3(a, b) register int a, b; { }
void h1(register volatile int a, b) { }
void h2(a, b) register volatile int a, b; { }
void h3(register struct { int a; } a, b) { }
void h4(a, b) register struct { int a; } a, b; { }
void h5(register int a[10], b[10]) { }
void h6(a, b) register int a[10], b[10]; { }
