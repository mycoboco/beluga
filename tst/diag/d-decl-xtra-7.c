typedef int x2, x3_1, x3_2, x5, x6_1, x6_2, x8, x9_1, x9_2, x10, x11_1, x11_2;
x2 int;                /* error */
x3_1 int, x3_2 int;    /* error */
void f4(void) {
    x5 int;                /* error */
    x6_1 int, x6_2 int;    /* error */
}
void f8(x8 int) { }                                 /* error */
void f9(x9_1 int, x9_2 int) { }                     /* error */
void f10(x10) x10 int; { }                          /* error */
void f11(x11_1, x11_2) x11_1 int, x11_2 int; { }    /* error */
