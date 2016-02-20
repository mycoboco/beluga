int x1; void f1(void) { extern int x1; }
int x2[1]; void f2(void) { int x2; { extern int x2[2]; } }
void f3_1(void) { extern int x3[1]; } void f3_2(void) { extern int x3[2]; }
void f4(void) { extern int x4[1]; extern int x4[]; } extern int x4[2];
void f5(void) { extern int x5[1]; { extern int x5[]; } } extern int x5[2];
extern int x6[10]; void f6(void) { long double *x6[2]; { extern int x6[11]; } }
static int x7[10]; void f7(void) { long double *x7[2]; { extern int x7[]; } }      /* linkage warning */
int x8_9[]; void f8(void) { long double *x8_9[2]; { extern int x8_9[2]; } }
            void f9(void) { long double *x8_9[2]; { extern int x8_9[3]; } }
extern int x10_11[10]; void f10_11(void) {
    extern int x10_11[]; { char x10_11; { extern int x10_11[11]; } } }
static int x12_13[10]; void f12_13(void) {
    extern int x12_13[]; { char x12_13; { extern int x12_13[10]; } } }             /* linkage warning */
extern int x14_16[];   void f14_16(void) {
    extern int x14_16[]; { char x14_16; { extern int x14_16[10]; } } }
    extern int x14_16[11];
int x17_18[10]; void f17_18_1(void) { extern int x17_18[]; }
                void f17_18_2(void) { extern int x17_18[]; sizeof(x17_18); }
int x19_20[]; void f19_20_1(void) { extern int x19_20[10]; }
              void f19_20_2(void) { extern int x19_20[11]; }
void f21_22_1(void) { extern int x21_22[10]; } int x21_22[];
void f21_22_2(void) { extern int x21_22[11]; }
static int x23_24;
void f23_24(void) { extern int x23_24; extern int x23_24; }        /* need to check linkage */
static int x25_26;
void f25_26(void) { extern int x25_26; { extern int x25_26; } }    /* need to check linkage */
