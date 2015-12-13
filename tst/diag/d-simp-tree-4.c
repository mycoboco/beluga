void f1(void)  { int *p1, a[1];    p1 = a - 1; p1 = a - 1U; p1 = a - 1l; p1 = a - 1Ul; }                    /* addrtree */
void f2(void)  { int *p2, a[1], i; p2 = a - (1+i); p2 = a - (1u+i); p2 = a - (1l+i); p2 = a - (1ul+i); }    /* addrtree */
void f3(void)  { int *p3, a[1], i; p3 = a - (i+1); p3 = a - (i+1u); p3 = a - (i+1L); p3 = a - (i+1UL); }    /* addrtree */
void f4(void)  { int *p4, a[1], i; p4 = a - (1-i); p4 = a - (1U-i); p4 = a - (1l-i); p4 = a - (1uL-i); }    /* addrtree */
void f5(void)  { int *p5, a[1], i; p5 = a - (i-1); p5 = a - (i-1u); p5 = a - (i-1l); p5 = a - (i-1UL); }    /* addrtree */
void f6(void)  { int *p6, a[1], i; p6 = (a+i) - 1; p6 = (a+i) - 1U; p6 = (a+i) - 1l; p6 = (a+i) - 1ul; }    /* addrtree */
void f7(void)  { int *p7, a[1], i; p7 = (a-i) - 1; p7 = (a-i) - 1u; p7 = (a-i) - 1L; p7 = (a-i) - 1UL; }    /* addrtree */
void f8(void)  { int *p8, i;       p8 = ((int *)8-i) - 1; p8 = ((int *)8-i) - 1ul; }    /* fold */
void f9(void)  { int *p9, i;       p9 = (i+(int *)8) - 1; p9 = (i+(int *)8) - 1ul; }    /* fold */
void f10(void) { int *p10, *x;     p10 = (x-1) - 2; p10 = (x-1Ul) - 2ul; }              /* fold */
void f11(void) { int *p11, *x;     p11 = (1+x) - 2; p11 = (1ul+x) - 2Ul; }              /* fold */

void f13(void) { int *p13; int i;  i = p13 - p13; }
void f14(void) { int i14;          i14 = (int *)8 - (int *)4; }      /* fold */
void f15(void) { int i15;          i15 = (int *)2 - (int *)1; }      /* fold */
void f16(void) { int i16;          i16 = (char *)2 - (char *)1; }    /* fold */
