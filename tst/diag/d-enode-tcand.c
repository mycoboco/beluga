void f1(void) { int i; double d; i = i && d; }
void f2(void) { int i; double d; i = (i > 0) && (d > 3.14); }
void f3(void) { int i; int *p1; void *p2; i = p1 && (p2 != p1); }
void f4(void) { struct tag { int x; } x; int i; i = x && x; }
