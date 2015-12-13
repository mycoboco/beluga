/* --std=c90 -Wv */
void f1(void) { int i; struct tag { int x; } x; int *p; i(); x(); p(); }    /* error */
void f2(void) { void h2(int, double); void g2(); h2(0, 1); g2(0, 1); }    /* prototype vs. non-prototype */
void f3(void) { struct tag { int x; } h3(void), x; int g3(void), y; x = h3(); y = g3(); }    /* struct vs. non-struct */
void f4(void) { struct tag h4(void); h4(); }    /* error - incomplete return */
void f5(void) { void h5(int, double); void g5(void); h5(0, 1); g5(); }    /* argument vs. no argument */
void f6(void) { struct tag; void h6(struct tag); struct tag *p; h6(*p); }    /* error - incomplete argument */
void f7(void) { void h7(int, ...); int x; h7(x, x, 0); }    /* arguments to variadic part */
void f8(void) { struct tag { int x; } x; void h8(int, struct tag, int *); h8(&x, x, &x); }    /* error - incompatible argument */
void f9(void) { void h9(char, short, int, long); char c; short s; int i; long l; h9(c, s, i, l); }    /* arguments that promote */
void f10(void) { void h10(int); h10(0, 1, 2); }    /* error - extra arguments */
void f11(void) { void h11(int, ...); char c; short s; float f; double d; h11(1, c, s, f, d); }    /* arguments to variadic part */
void f12(void) { void h12(int, ...); struct tag *p; h12(0, *p, *p); }    /* incomplete arguments to variadic part */
void f13(void) { struct tag { int x; }; struct tag h13(void); void g13(struct tag); g13(h13()); }    /* function call to struct argument */
void f14(void) { struct tag { int x; } *p; void h14(struct tag); h14(*p); }    /* struct argument */
void f15(void) { int h15(void); void g15(int); g15(h15()); }    /* function call to argument */
void f16(void) { int h16(int, double); h16(1); }    /* error - insufficient argument */
void f17(void) { void (*h17())(void); h17()(); }    /* function call to function expression */
void f18(void) { char h18(void); short g18(void); h18(); g18(); }    /* small return types */
void f19(void) { enum { X } h19(void); h19(); }    /* enum return type */
void f20(void) { void *h20(void), *p; p = h20(); }    /* pointer return type */
void f21(void) { void h21(); h21(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31); }
void f22(void) { void h22(); h22(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32); }    /* warning */
