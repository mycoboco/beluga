void f1(void) { static void f2(void); }    /* error */
void f2(void) { auto void f3(void); }      /* error */
void f3(void) { register volatile int x; }                             /* warning */
void f4(void) { register struct { int m; } x; register int y[10]; }    /* warning */
void f5(int x) { int x; }                                              /* redeclaration of x */
void f6(int x) { extern int x; }                                       /* redeclaration of x */
void f7(int x) { static int x; extern int x; int y; extern int y; }    /* redeclaration of x */
void f8(void) { static char x[]; struct tag y; }    /* error */
void f9(void) { static int x = 0; static int y[] = { 1, 2 }; sizeof(y); }
void f10(void) { static int x = 0; = 1; }    /* error */
