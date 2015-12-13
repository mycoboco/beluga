int A1; void f1(enum { A1 } p1);                 /* hide */
int A2; void f2(enum { A2 } p2) {}               /* hide */
int A3; void f3(void) { enum { A3 } p3; }        /* hide */
void f4(int A4) { enum { A4 } p4; }              /* redecl */
void f5(int A5) { { enum { A5 } p5; } }          /* hide */
void f6(void) { int A6; { enum { A6 } p6; } }    /* hide */

int p8; void f8(int p8) {}                                    /* hide */
int p9; void f9(double p9) {}                                 /* hide */
void f10(int p10) { int p10; }                                /* redecl */
void f11(int p11) { void f112(int p11); }                     /* hide? */
void f12(int p12) { { void f122(double p12); } }              /* hide? */
int p13; void f13(p13) {}                                     /* hide */
int p141, p142; void f14(p141) double p141, p142; {}          /* hide */
void f15(p15) { int p15; }                                    /* redecl */
void f16(void) { int p16; void f162(int p16); }               /* hide? */
void f17(void) { int p17; { void f172(int p17); } }           /* hide? */
void f18(void) { void f182(int p18); void f192(int p18); }

int t20; void f20(typedef int t20);                            /* hide? */
int t21; void f21(typedef double t21);                         /* hide? */
int t22; void f22(void) { typedef int t22; }                   /* hide */
int t23; void f23(void) { typedef double t23; }                /* hide */
void f24(typedef int t24) { typedef int t24; }
void f25(int t25) { typedef int t25; }                         /* redecl */
void f26(int t26) { { typedef int t26; } }                     /* hide */
void f27(void) { int t27; { typedef int t27; } }               /* hide */
void f28(void) { typedef double t28; { typedef int t28; } }    /* hide */

extern int x30; void f30(void) { extern int x30; }
extern int x31; void f31(void) { static int x31; }                 /* hide */
extern int x32; void f32(void) { int x32; }                        /* hide */
static int x33; void f33(void) { extern int x33; }
static int x34; void f34(void) { static int x34; }                 /* hide */
static int x35; void f35(void) { int x35; }                        /* hide */
int x36;        void f36(void) { extern int x36; }
int x37;        void f37(void) { static int x37; }                 /* hide */
int x38;        void f38(void) { int x38; }                        /* hide */
void f39(void) { extern int x39; { extern int x39; } }
void f40(void) { extern int x40; { static int x40; } }             /* hide */
void f41(void) { extern int x41; { int x41; } }                    /* hide */
void f42(void) { static int x42; { extern int x42; } }             /* hide */
void f43(void) { static int x43; { static int x43; } }             /* hide */
void f44(void) { static int x44; { int x44; } }                    /* hide */
void f45(void) { int x45; { extern int x45; } }                    /* hide */
void f46(void) { int x46; { static int x46; } }                    /* hide */
void f47(void) { int x47; { int x47; } }                           /* hide */
int x48; void f48(void) { extern int x48; { extern int x48; } }
int x49; void f49(void) { static int x49; { extern int x49; } }    /* hide */
int x50; void f50(void) { int x50; { extern int x50; } }           /* hide */
typedef int x51; void f51(void) { extern int x51; }                /* hide */
typedef int x52; void f52(void) { static int x52; }                /* hide */
typedef int x53; void f53(void) { int x53; }                       /* hide */
enum { x54 } z54; void f54(void) { extern int x54; }               /* hide */
enum { x55 } z55; void f55(void) { static int x55; }               /* hide */
enum { x56 } z56; void f56(void) { int x56; }                      /* hide */
