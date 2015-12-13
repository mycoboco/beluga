void f1(void) { unsigned u;      u = 3U & 4u; }      /* fold */
void f2(void) { unsigned long m; m = 3Ul & 4uL; }    /* fold */
void f3(void) { unsigned u;      u = u & 0xFFFFFFFF; }    /* identity */
void f4(void) { unsigned long m; m = m & 0xFFFFFFFF; }    /* identity */
void f5(void) { unsigned u;      u = u & 0; }    /* no effect */
void f6(void) { unsigned long m; m = m & 0; }    /* no effect */

void f8(void)  { unsigned u;      u = 3U | 4u; }      /* fold */
void f9(void)  { unsigned long m; m = 3ul | 4UL; }    /* fold */
void f10(void) { unsigned u;      u = u | 0; }    /* identity */
void f11(void) { unsigned long m; m = m | 0; }    /* identity */

void f13(void) { unsigned u;      u = 3u ^ 4U; }      /* fold */
void f14(void) { unsigned long m; m = 3ul ^ 4UL; }    /* fold */
void f15(void) { unsigned u;      u = u ^ 0; }    /* identity */
void f16(void) { unsigned long m; m = m ^ 0; }    /* identity */
