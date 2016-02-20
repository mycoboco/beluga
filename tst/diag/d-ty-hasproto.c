struct t1 { int (*f)(); };
void f2(int (*)());
struct t3 { void (*f3)(int (*)()); };
void f4(void) { sizeof(int (*)()); }
void f5();
void f6(int);
void f7(a, b) { }
void f8(int a, int b) { }
typedef void (*fp_t)();
fp_t x10;
void f11(struct t1 *x);
void f12(struct { int (*f)(); } *x);
