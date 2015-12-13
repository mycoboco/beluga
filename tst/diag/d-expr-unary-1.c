void f1(void) { int *p; int x; x = *p; x = *x; }    /* error */
void f2(void) { register int x; int *p; p = &x; p = &p; }    /* error */
void f3(void) { int x, *p; x = +x; p = &+x; }    /* error */
void f4(void) { int x; unsigned y; x = -x; x = -y; }    /* warning */
void f5(void) { int x; unsigned y; x = ~x; x = ~y; }
void f6(void) { int x; x = !x; x = !!x; }
void f7(void) { int x; ++x; }
void f8(void) { int x; sizeof(int); sizeof(int *); sizeof x; sizeof &x; sizeof(x, &x); }
void f9(void) { struct { int x:1; } x; sizeof(int []); sizeof x.x; sizeof(x, x.x); }    /* error */
int x10 = -2147483648;    /* warning */
