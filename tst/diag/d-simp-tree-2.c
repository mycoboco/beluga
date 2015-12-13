void f1(void)     { int *p,  a[1];     p = a + 1; p = a + 1U; p = a + 1l; p = a + 1ul; }    /* addrtree */
void f2_3(void)   { int *p2, a[1], i;  p2 = (i+1) + a; p2 = (i+1u) + a; p2 = (i+1L) + a; p2 = (i+1ul) + a;
                                       p2 = a + (1+i); p2 = a + (1u+i); p2 = a + (1l+i); p2 = a + (1ul+i); }    /* addrtree */
void f4_5(void)   { int *p4, a[1], i;  p4 = (i-1) + a; p4 = (i-1u) + a; p4 = (i-1l) + a; p4 = (i-1UL) + a;
                                       p4 = a + (1-i); p4 = a + (1u-i); p4 = a + (1l-i); p4 = a + (1Ul-i); }    /* addrtree */
void f6_7(void)   { int *p6, a[1], i;  p6 = (a+i) + 1; p6 = (a+i) + 1u; p6 = (a+i) + 1l; p6 = (a+i) + 1ul;
                                       p6 = 1 + (i+a); p6 = 1u + (i+a); p6 = 1l + (i+a); p6 = 1uL + (i+a); }    /* addrtree */
void f8_9(void)   { int *p8, a[1], i;  p8 = (a-i) + 1; p8 = (a-i) + 1u; p8 = (a-i) + 1L; p8 = (a-i) + 1ul;
                                       p8 = 1 + (a-i); p8 = 1U + (a-i); p8 = 1L + (a-i); p8 = 1ul + (a-i); }    /* addrtree */
void f10_11(void) { int *p10, i;       p10 = (i+(int *)1) + 1; p10 = (i+(int *)1) + 1ul;
                                       p10 = 1 + ((int *)1+i); p10 = 1UL + ((int *)1+i); }                      /* addrtree */
void f12_13(void) { int *p12, i;       p12 = ((int *)1-i) + 1; p12 = ((int *)1-i) + 1Ul;
                                       p12 = 1 + ((int *)1-i); p12 = 1uL + ((int *)1-i); }                      /* addrtree */
void f14_15(void) { int *p14, *x;      p14 = (x+1) + 2; p14 = (x+1ul) + 2ul;
                                       p14 = 2 + (1+x); p14 = 2ul + (1uL+x); }                                  /* fold */
void f16_17(void) { int *p16, *x;      p16 = (x-1) + 2; p16 = (x-1UL) + 2UL;
                                       p16 = 2 + (x-1); p16 = 2ul + (x-1ul); }                                  /* fold */
