typedef int x2, x3_1, x3_2, x5, x6_1, x6_2, x8, x9_1, x9_2, x10, x11_1, x11_2;
int x2;            /* error */
int x3_1, x3_2;    /* error */
void f4(void) {
    int x5;
    int x6_1, x6_2;
}
void f8(int x8) { }
void f9(int x9_1, x9_2) { }    /* error */
void f10(x10) int x10; { }    /* error */
void f11(x11_1, x11_2) int x11_1, x11_2; { }    /* error */
