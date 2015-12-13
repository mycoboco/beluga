x1;            /* warning */
x2_1, x2_2;    /* warning */
void f3(void) {
    x4;            /* error */
    x5_1, x5_2;    /* error */
}
void f7(x7) { }
void f8(x8_1, x8_2) { }
void f9(x9) x9; { }                        /* error */
void f10(x10_1, x10_2) x10_1, x10_2 { }    /* error */
