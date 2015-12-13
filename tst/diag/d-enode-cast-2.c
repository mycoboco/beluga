void f1(void) { char c; short s; int i; c = i; s = i; i = c; i = s; c = s; }
void f2(void) { char c; short s; long l; c = l; s = l; l = c; l = s; s = c; }
void f3(void) { unsigned char c; unsigned short s; unsigned u; c = u; s = u; u = c; u = s; c = s; }
void f4(void) { unsigned char c; unsigned short s; unsigned long u; c = u; s = u; u = c; u = s; s = c; }
void f5(void) { int i; long l; unsigned u; unsigned long ul; l = i; u = i; ul = i; }
void f6(void) { long l; int i; unsigned u; unsigned long ul; i = l; u = l; ul = l; }
void f7(void) { unsigned u; int i; long l; unsigned long ul; i = u; l = u; ul = u; }
void f8(void) { unsigned long ul; int i; long l; unsigned u; i = ul; l = ul; u = ul; }
void f9(void) { void *p; unsigned u; unsigned long ul; u = (unsigned)p; ul = (unsigned long)p; p = (void *)u; p = (void *)ul; }
void f10(void) { void *p; int *pi; void (*f)(void); p = (void *)pi; pi = (int *)p; p = (void *)f; f = (void (*)(void))p; }    /* warning */
