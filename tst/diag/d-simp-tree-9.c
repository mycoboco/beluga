void f1(void) { int i;  i = ~7; }     /* fold */
void f2(void) { long l; l = ~0L; }    /* fold */
void f3(void) { int i;  i = ~~i; }    /* idempotent */
void f4(void) { long l; l = ~~l; }    /* idempotent */
void f5(void) { unsigned u;      u = ~7u; }     /* fold */
void f6(void) { unsigned long m; m = ~0Ul; }    /* fold */
void f7(void) { unsigned u;      u = ~~u; }    /* idempotent */
void f8(void) { unsigned long m; m = ~~m; }    /* idempotent */
