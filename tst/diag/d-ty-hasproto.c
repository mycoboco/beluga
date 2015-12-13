struct t1 { int (*f)(); };    /* warning */
void f2(int (*)());    /* warning */
struct t3 { void (*f3)(int (*)()); };    /* warning */
void f4(void) { sizeof(int (*)()); }    /* warning */
void f5();    /* warning */
void f6(int);
void f7(a, b) { }    /* warning */
void f8(int a, int b) { }
typedef void (*fp_t)();    /* warning */
fp_t x10;                  /* warning */
void f11(struct t1 *x);
void f12(struct { int (*f)(); } *x);    /* warning */
