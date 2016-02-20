int f(void) { return sizeof(int *p); }
void f(void) { int * const p; p = 0; }
void f3(int *()); void f3(int *x(int)); void f3(int *(x));
typedef double f4_t; void f4(int (f4_t)); void f4(int (double));
void f5(void) { sizeof(int (f4_t)); }
typedef double f6_t; int (f6_t);
typedef double f7_t; void f7(void) { int (f7_t); f7_t = 0; }
void f8(int (*)); void f8(int *); void f8(int ());
void f9(int (sizeof)); void f9(int ());
int (f10); int f10; int f10();
int f11_1(f11_1);
void (int);
int f13[0];
int f14[-1];
int f15[];
int f16[int f16(void)];
int f17[0x80000000];
int f18[0x80000000*1+1];
