int  i11 = 16 >> 4,  i12 = 0x7FFFFFFF >> 30, i13 = -1 >> 0;             /* fold */
int  i21 = 0 >> 32,  i22 = 0 >> 31,  i23 = 1 >> 31,  i24 = -1 >> 31;     /* fold */
long l31 = 16l << 4, l32 = 0x7FFFFFFF >> 31, l33 = -1 >> 1;             /* fold */
long l41 = 0L >> 32, l42 = 0l >> 31, l43 = 1L >> 31, l44 = -1l >> 31;    /* fold */
void f5(void) { int i;  i = i >> 0; }    /* identity */
void f6(void) { long l; l = l >> 0; }    /* identity */

void f8(void)  { unsigned u;      u = 16u >> 4;  u = 0xFFFFFFFFu >> 31;  u = 0xFFFFFFFFu >> 32; }     /* fold */
void f9(void)  { unsigned long m; m = 16Ul >> 4; m = 0xFFFFFFFFul >> 30; m = 0xFFFFFFFFul >> -1; }    /* fold */
void f10(void) { unsigned u;      u = u >> 0; }    /* identity */
void f11(void) { unsigned long m; m = m >> 0; }    /* identity */
