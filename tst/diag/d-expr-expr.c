void f1(void) { int a, b, c; a = b, b = c; }
void f2(void) { int a, b, c; a = b, b = c, c = a; }
void f3(void) { int a, b, c; a = b, (b = c, c = a); }
