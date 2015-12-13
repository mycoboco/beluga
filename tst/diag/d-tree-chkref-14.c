void f1() { int x; f1(x, x, x); }
void f2() { int x; f2(x, x, &x); }
void f3() { int x; f3(x, &x, x); }
void f4() { int x; f4(&x, x, x); }
void f5() { int x; f5(&x, &x, &x); }
