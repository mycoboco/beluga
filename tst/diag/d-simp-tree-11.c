int  i11 = 3 << 4,   i12 = 2 << 30,  i13 = -1 << 0;                      /* fold */
int  i21 = 0 << 32,  i22 = 0 << 31,  i23 = 1 << 31,  i24 = -1 << 31;     /* fold */
long l31 = 3L << 4,  l32 = 1l << 31, l33 = -1 << 1;                      /* fold */
long l41 = 0l << 32, l42 = 0L << 31, l43 = 1l << 31, l44 = -1l << 31;    /* fold */
void f5(void) { int i;  i = i << 0; }    /* identity */
void f6(void) { long l; l = l << 0; }    /* identity */

void f8(void)  { unsigned u;      u = 3U << 4;  u = 1u << 31; u = 1u << 32; }    /* fold */
void f9(void)  { unsigned long m; m = 3uL << 4; m = 2u << 30; m = 1u << -1; }    /* fold */
void f10(void) { unsigned u;      u = u << 0; }    /* identity */
void f11(void) { unsigned long m; m = m << 0; }    /* identity */
