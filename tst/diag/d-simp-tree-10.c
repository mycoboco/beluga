void f1(void) { float f;        f = -3.141592F; }     /* fold */
void f2(void) { double d;       d = -3.141592; }      /* fold */
void f3(void) { long double ld; ld = -3.141592l; }    /* fold */
void f4(void) { float f;        f = -(-f); }      /* idempotent */
void f5(void) { double d;       d = -(-d); }      /* idempotent */
void f6(void) { long double ld; ld = -(-ld); }    /* idempotent */

void f8(void)  { int i;  i = -3;  i = -(-0x7FFFFFFF-1); }     /* fold */
void f9(void)  { long l; l = -3L; l = -(-0x7FFFFFFFl-1); }    /* fold */
void f10(void) { int i;  i = -(-i); }    /* idempotent */
void f11(void) { long l; l = -(-l); }    /* idempotent */
