int  i11 = 3 % 4,   i12 = (-2147483647-1) % -1;      /* fold */
long l11 = 3l % 4L, l12 = (-2147483647L-1) % -1l;    /* fold */
void f3(void) { int a;  a = a % 0; }     /* divide by 0 */
void f4(void) { int a;  a = a % 0l; }    /* divide by 0 */
void f5(void) { int a;  a = a % 1; }
void f6(void) { long a; a = a % 1L; }
void f7(void) { int a;  a = a % -1; }
void f8(void) { long a; a = a % -1l; }

void f10(void) { unsigned u;      u = 3u % 4u;   u = 0xFFFFFFFFU % -1; }     /* fold */
void f15(void) { unsigned long m; m = 3Ul % 4uL; m = 0xFFFFFFFFUL % -1; }    /* fold */
void f16(void) { unsigned u;      u = u % 1u; }     /* no effect */
void f17(void) { unsigned long m; m = m % 1Ul; }    /* no effect */
void f18(void) { unsigned u;      u = u % 0u; }     /* divide by 0 */
void f19(void) { unsigned long m; m = m % 0uL; }    /* divide by 0 */
void f20(void) { unsigned u;      u = u % 8U; }      /* to bitwise AND */
void f21(void) { unsigned long m; m = m % 16UL; }    /* to bitwise AND*/
