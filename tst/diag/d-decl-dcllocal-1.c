void f1(void) { static void f2(void); }
void f2(void) { auto void f3(void); }
void f3(void) { register volatile int x; }
void f4(void) { register struct { int m; } x; register int y[10]; }
void f5(int x) { int x; }                                              /* redeclaration of x */
void f6(int x) { extern int x; }                                       /* redeclaration of x */
void f7(int x) { static int x; extern int x; int y; extern int y; }    /* redeclaration of x */
void f8(void) { static char x[]; struct tag y; }
void f9(void) { static int x = 0; static int y[] = { 1, 2 }; sizeof(y); }
void f10(void) { static int x = 0; = 1; }
