struct tag { int x; };
void f2(static int a, register volatile int b, register struct tag c);                /* error, warning */
void f3(a, b, c) static int a; register volatile int b; register struct tag c; { }    /* error, warning */
void f4(int a, int a);             /* error */
void f5(float a, char *a) { }      /* error */
void f6(a, a) { }                  /* error */
void f7(a) int a; double a; { }    /* error */
void f8(int a = 12, int b, char a) { }    /* error */
void f9(typedef int a, typedef int b) { }    /* error */
void f10(typedef int a);                     /* error */
void f11(a, b) typedef int a, b; { }         /* error */
