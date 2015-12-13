float       F11 = 3.14F * 3.14f, F12 = -1.17549e-38F * 2.0f,  F13 = 3.40282e+38f * 2.0f,   F14 = -1.17549e-38f * -3.0f;    /* fold */
double      d21 = 3.14 * 3.14,   d22 = -2.22507e-308 * 2.0,   d23 = 1.79769e+308 * 2.0,    d24 = -2.22507e-308 * -3.0;     /* fold */
long double x31 = 3.14L * 3.14l, x32 = -1.18973e+4932L *3.0l, x33 = 1.18973e+4932l * 2.0L, x34 = -1.18973e+4932;           /* fold */
int  i41 = 3 * 4,   i42 = -2147483647 * 2,   i43 = 2147483647 * 2, i44 = (-2147483647-1) * -1;    /* fold */
long l51 = 3l * 4L, l52 = -2147483647L * 2l, l53 = 2147483647l * 2l;                              /* fold */

void f7(void)  { int a, i;  i = a * 1; }     /* identity */
void f8(void)  { long a, l; l = a * 1L; }    /* identity */
void f9(void)  { int a;     a = a * 0; }     /* no effect */
void f10(void) { int a;     a = a * 0l; }    /* no effect */
void f11(void) { int a;     a = a * 8; }      /* to shift */
void f12(void) { long a;    a = a * 16L; }    /* to shift */

void f14(void) { unsigned u;      u = 3U * 4u;   u = 0xFFFFFFFF * 2; }      /* fold */
void f15(void) { unsigned long m; m = 3Ul * 4uL; m = 0xFFFFFFFFUL * 2; }    /* fold */
void f16(void) { unsigned u;      u = u * 1U; }     /* identity */
void f17(void) { unsigned long m; m = m * 1ul; }    /* identity */
void f18(void) { unsigned u;      u = u * 0u; }     /* no effect */
void f19(void) { unsigned long m; m = m * 0UL; }    /* no effect */
void f20(void) { unsigned u;      u = u * 8u; }      /* to shift */
void f21(void) { unsigned long m; m = m * 16ul; }    /* to shift */

int i231 = 2 * -1, i232 = -2 * -1, i233 = 2 * -2, i234 = -2 * -2;               /* fold */
long l241 = 2 * -1l, l242 = -2 * -1L, l243 = 2 * -2l, l244 = -2 * -2L;          /* fold */
double d251 = 2.0 * -1, d252 = -2.0 * -1, d253 = 2.0 * -2, d254 = -2.0 * -2;    /* fold */
