typedef int x2, x3_1, x3_2, x5, x6_1, x6_2, x8, x9_1, x9_2, x10, x11_1, x11_2;
x2;            /* error */
x3_1, x3_2;    /* error */
void f4(void) {
    x5;            /* error */
    x6_1, x6_2;    /* error */
}
void f8(x8) { }            /* error */
void f9(x9_1, x9_2) { }    /* error */
void f10(x10) x10; { }                     /* error */
void f11(x11_1, x11_2) x11_1, x11_2 { }    /* error */
