int f(void) { return sizeof(int *p); }    /* error */
void f(void) { int * const p; p = 0; }    /* error */
void f3(int *()); void f3(int *x(int)); void f3(int *(x));    /* error */
typedef double f4_t; void f4(int (f4_t)); void f4(int (double));
void f5(void) { sizeof(int (f4_t)); }    /* error */
typedef double f6_t; int (f6_t);    /* error */
typedef double f7_t; void f7(void) { int (f7_t); f7_t = 0; }
void f8(int (*)); void f8(int *); void f8(int ());    /* error */
void f9(int (sizeof)); void f9(int ());    /* error */
int (f10); int f10; int f10();    /* error */
int f11_1(f11_1);    /* error */
void (int);    /* error */
int f13[0];     /* error */
int f14[-1];    /* error */
int f15[];
int f16[int f16(void)];    /* error */
int f17[0x80000000];    /* error */
int f18[0x80000000*1+1];    /* error */
