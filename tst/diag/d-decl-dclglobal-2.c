extern int x1[10];  extern int x1[20];    /* redeclaration of x1 */
extern double x2[]; extern double x2[10];
void f3(void) { extern int x3[]; } static int x3[1];    /* linkage warning */
void f4(void) { extern int x4[]; } int x4[1];
extern int x5[10]; void f5(void) { int x5; { extern int x5[20]; } }   int x5[10];          /* warning */
static int x6[];   void f6(void) { int x6; { extern int x6[5]; } }    static int x6[7];    /* error, linkage warning */
extern int x7[];   void f7(void) { extern int x7[10]; }               int x7[20];          /* warning */
extern int x8[];   void f8(void) { float x8; { extern int x8[10]; } } int x8[20];          /* warning */
extern int x9;  void f9(void)  {  extern int x9;  }  extern int x9;
extern int x10; void f10(void) {  extern int x10; }  static int x10;     /* linkage error */
extern int x11; void f11(void) {  extern int x11; }  int x11;
static int x12; void f12(void) {  extern int x12; }  extern int x12;
static int x13; void f13(void) {  extern int x13; }  static int x13;
static int x14; void f14(void) {  extern int x14; }  int x14;            /* linkage error */
int x15;        void f15(void) {  extern int x15; }  extern int x15;
int x16;        void f16(void) {  extern int x16; }  static int x16;     /* linkage error */
int x17;        void f17(void) {  extern int x17; }  int x17;
void f18(void) { extern int a[10]; { extern int a[]; sizeof(a); }   sizeof(a); }
void f19(void) { extern int a[];   { extern int a[10]; sizeof(a); } sizeof(a); }               /* error */
void f20(void) { extern int a[10]; { int a; { extern int a[];   sizeof(a); } } sizeof(a); }    /* error */
void f21(void) { extern int a[];   { int a; { extern int a[10]; sizeof(a); } } sizeof(a); }    /* error */
static void f22(void); void f22(void) { }
static void f23(void); extern void f23(void) { }
