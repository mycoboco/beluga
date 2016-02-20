struct tag { int x; };
void f2(static int a, register volatile int b, register struct tag c);
void f3(a, b, c) static int a; register volatile int b; register struct tag c; { }
void f4(int a, int a);
void f5(float a, char *a) { }
void f6(a, a) { }
void f7(a) int a; double a; { }
void f8(int a = 12, int b, char a) { }
void f9(typedef int a, typedef int b) { }
void f10(typedef int a);
void f11(a, b) typedef int a, b; { }
