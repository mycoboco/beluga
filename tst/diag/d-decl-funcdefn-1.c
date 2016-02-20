struct tag f1(void) { }
void f2_5(int, int, int, unsigned, long, unsigned long, double, double, long double, char *);
void f2_5(a, b, c, d, e, f, g, h, i, j)
char a; unsigned short b; int c; unsigned d; long e; unsigned long f; float g; double h; long double i; char *j;
{ }
void f6(char); void f6(a) char a; { }
void f7(char, short); void f7(a, b) char a; short b; { }
void f8(int); void f8(a, b) char a; { }
void f9(double, int); void f9(a) float a; { }
void f10(); void f10() { }
void f11(int); void f11(a) char a; { }
void f12(struct tag x, int []); void f12(struct tag a, int b[]) { }
void f13(struct tag x, int x[]) { }
void f14(struct tag) { }
void f15(a, b) struct tag a; int b[]; { }
void f16(a, b) int a(void); int b[1]; { }
void f17(int a) { } void f17(int a) { }
void f18(void) { }
int f19(void) { }
double f20(void) { }
int *f21(void) { lab: goto lab; }
