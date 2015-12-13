void f1(void) { int i, x; x = i++; x = --i; }
int f2(int *a) { a[0] = a[1]; return a[1]; }
void f3(void) { int a[10], x; x = f2(a); }
struct tag { char c[5]; int x; }; void f4(void) { int y; struct tag x, *px = &x; y = x.x; y = px->x; }
