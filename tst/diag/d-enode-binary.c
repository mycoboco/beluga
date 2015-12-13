void f1(void) { long double x; char c; c = x + c; }    /* long double */
void f2(void) { double d; int i; i = d + i; }    /* double */
void f3(void) { float f; short s; f = f + s; }    /* float */
void f4(void) { unsigned long ul; unsigned char uc; uc = ul + uc; }    /* unsigned long */
void f5(void) { long l; unsigned u; u = l + u; u = u + l; }    /* unsigned long (int == long) */
void f6(void) { long l; l = l + l; }    /* long */
void f7(void) { unsigned char uc; char c; uc = uc + c; }    /* int */
