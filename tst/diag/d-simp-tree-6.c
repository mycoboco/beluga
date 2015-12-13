float       F11 = 3.14F / 3.14f, F12 = -1.17549e-38f / 2.0F,  F13 = 3.40282e+38f / 0.5f,    F14 = -1.17549e-38F / -0.3f;     /* fold */
double      d21 = 3.14 / 3.14,   d22 = -2.22507e-308 / 2.0,   d23 = 1.79769e+308 / 0.5,     d24 = -2.22507e-308 / -0.3;      /* fold */
long double x31 = 3.14L / 3.14l, x32 = -1.18973e+4932L / 3.0l, x33 = 1.18973e+4932l / 0.5L, x34 = -1.18973e+4932 / -0.3L;    /* fold */
int  i41 = 3 / 4,   i42 = (-2147483647-1) / -1;      /* fold */
long l51 = 3l / 4L, l52 = (-2147483647L-1) / -1l;    /* fold */

void f7(void)  { int a, i;  i = a / 1; }     /* identity */
void f8(void)  { long a, l; l = a / 1L; }    /* identity */
void f9(void)  { int a;     a = a / 0; }     /* divide by 0 */
void f10(void) { int a;     a = a / 0l; }    /* divide by 0 */
void f11(void) { int a;     a = a / 8; }      /* no to shift */
void f12(void) { long a;    a = a / 16L; }    /* no to shift */

void f14(void) { unsigned u;      u = 3U / 4u;   u = 0xFFFFFFFFU / -1; }     /* fold */
void f15(void) { unsigned long m; m = 3Ul / 4uL; m = 0xFFFFFFFFUL / -1; }    /* fold */
void f16(void) { unsigned u;      u = u / 1u; }     /* identity */
void f17(void) { unsigned long m; m = m / 1UL; }    /* identity */
void f18(void) { unsigned u;      u = u / 0u; }     /* divide by 0 */
void f19(void) { unsigned long m; m = m / 0Ul; }    /* divide by 0 */
void f20(void) { unsigned u;      u = u / 8U; }      /* to shift */
void f21(void) { unsigned long m; m = m / 16ul; }    /* to shift */
