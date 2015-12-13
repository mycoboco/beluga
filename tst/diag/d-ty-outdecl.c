/* --proto -Wv --std=c90 */
typedef struct { int x; } x2_t; const x2_t * const f2(const x2_t x) { &x; return &x; }
typedef struct x3_t x3_t; x3_t * volatile f3(x3_t * volatile x) { &x; return x; }
struct t4 { int x; double *y; } *x4(void) { return 0; }
struct t4 * const x5(struct t4 p5) { &p5; return 0; }
struct { int x; double *y; } *x6(void) { return 0; }                       /* warning */
struct { int x; double *y; } *x7(struct { int x; } x) { &x; return 0; }    /* warning */
struct { int x; } *x8;                                                     /* warning */
void *x9_1; void x9_2;
float x10_1; float * const x10_2;
double x11_1; double x11_2(double *x11_3);
long double x12_1; long double *x12_2;
char x13_1; char *x13_2; const char *x13_3;
signed char x14_1; signed char *x14_2;
unsigned char x15_1; unsigned char *x15_2;
short x16_1; short *x16_2;
signed short x17_1; volatile signed short *x17_2;
unsigned short x18_1; const unsigned short * volatile x18_2;
int x19_1; int *x19_2;
signed int x20_1; signed int *x20_2;
unsigned int x21_1; unsigned int *x21_2;
long x22_1; long *x22_2;
signed long x23_1; const signed long *x23_2;
unsigned long x24_1; unsigned long * const x24_2;
typedef double **ppdbl_t; double **x25;
typedef char *string; string x26;
int *x27_1(void); int (*x27_2)(void);
int *x28_1[10]; int (*x28_2)[10];
void x29(int a, ...) { &a; }
void x30(a, b, c) char a; float b; const double c; { &a, &b, &c; }
void x31(int a()) { &a; }
void x32() { }
int x33[1][2][3];
int (* volatile (*x34[1])[2])[3];
int x35[][2];
int x36[][];    /* error */
int x37[];      /* error */
void main(struct { int x; } x) {}
