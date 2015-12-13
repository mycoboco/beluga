void f1(void) { extern double f2_3(void); char *x; 1; 3.14; x = "abcd"; }
void f2(void) { int x; x = f2_2(); x = f2_3(); }    /* warning */
void f3(void) { int *x = &x3; }    /* error */
enum { X4 = 1 }; void f4(void) { int x; x = X4; }
void f5(void) { typedef int x5; int x; x = x5; }    /* error */
void f6(void) { int x; x = int; }    /* error */
void f7(void) { 1 / (1 - if); }    /* error */
void f8(void) { if (1 * (1 / (1 - if)) + 1) return; }    /* error */
void f9(void) { while (1 * (1 / )); }    /* error */
void f10(void) { do; while((1 / ) * 1); }    /* error */
void f11(void) { for (1 / (1-if); (1 / (1-; 1 / (1-); }    /* error */
void f12(void) { switch(1 / (1-{)) { case 1 / (1-case): ; } }    /* error */
int f13(void) { return 1 / (1-return); }    /* error */
void f14(int x = 1 / (1-int)) {}    /* error */
void f15(void) { int x = { 1 / (1-) }; int y = 1 / (1-void); }    /* error */
void f16(void) { char c[] = { 0, 1, 2, 1 / (1-char), 3, 4 }; }    /* error */
int x17 = 1 / (1 - &);    /* error */
void f18(void) { int x[1 / (1-[])]; }    /* error */
void f19(void) { struct { int x: 1 / (1-default); int y; } z; }    /* error */
enum { A = 0, B = 1 / (1-enum), C, D };    /* error */
void f20(void) { f(0, f(f(1 / (1-[]), 2), 3)); }    /* error */
