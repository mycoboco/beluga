typedef int x2, x3_1, x3_2, x5, x6_1, x6_2, x8, x9_1, x9_2, x10, x11_1, x11_2;
int x2 y2;                   /* error */
int x3_1 y3_1, x3_2 y3_2;    /* error */
void f4(void) {
    int x5 y5; {                   /* error */
    int x6_1 y6_1, x6_2 y6_2; }    /* error */
}
void f8(int x8 y8) { }                                     /* error */
void f9(int x9_1 y9_1, int x9_2 y9_2) { }                  /* error */
void f10(x10) int x10 y10; { }                             /* error */
void f11(x11_1, x11_2) int x11_1 y11_1, x11_2 y11_2 { }    /* error */
