void f1(void) { float f; unsigned u; f = u; u = f; }
void f2(void) { double d; unsigned long ul; d = ul; ul = d; }
void f3(void) { float f; unsigned char uc; uc = f; f = uc; }
void f4(void) { void *p; double d; d = (double)p; p = (void *)d; }    /* error */
void f5(void) { unsigned char uc; short s; uc = s; s = uc; }
