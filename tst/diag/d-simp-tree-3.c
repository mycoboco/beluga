float       F11 = 3.14F - 3.14f, F12 = -1.17549e-38f - 1.17549e-38F, F13 = 3.40282e+38f - 3.40282e+38f;    /* fold */
double      d21 = 3.14 - 3.14,   d22 = -2.22507e-308 - 2.22507e-308, d23 = 1.79769e+308 - 1.79769e+308;    /* fold */
long double x31 = 3.14l - 3.14L, x32 = -1.18973e+4932l - 1.18973e+4932L - 1.18973e+4932l,
            x33 = 1.18973e+4932L - 1.18973e+4932L;                                                         /* fold */
int  i51 = 3 - 4,   i52 = -2147483647 - 2,   i53 = -2147483647 - 1;      /* fold */
long l61 = 3l - 4l, l62 = -2147483647l - 2l, l63 = -2147483647l - 1l;    /* fold */
void f7(void) { int a, i;  i = a - 0; }     /* identity */
void f8(void) { long a, l; l = a - 0L; }    /* identity */

void f10(void) { unsigned u;      u = 3U - 4u;   u = -0xFFFFFFFF - 2; }      /* fold */
void f11(void) { unsigned long m; m = 3ul - 4UL; m = -0xFFFFFFFFUL - 2; }    /* fold */
void f12(void) { unsigned u;      u = u - 0u; }     /* identity */
void f13(void) { unsigned long m; m = m - 0Ul; }    /* identity */

void f15(void) { int *p; p = (int *)4 - 1;   }    /* fold */
void f16(void) { int *p; p = (int *)4 - 1u;  }    /* fold */
void f17(void) { int *p; p = (int *)4 - 1l;  }    /* fold */
void f18(void) { int *p; p = (int *)4 - 1uL; }    /* fold */
void f19(void) { int *p; p = p - 0; p = p - 0u; p = p - 0l; p = p - 0UL; }    /* identity */
