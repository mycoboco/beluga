void f1(void)  { long double ld;   ld = (long double)2147483647L; }                                           /* L to X */
void f2(void)  { char c1; static char c2 = (char)255l;             c1 = (char)127L; }                         /* L to C */
void f3(void)  { short s1; static short s2 = (short)32768l;        s1 = (short)32767l; }                      /* L to S */
void f4(void)  { int i;            i = (int)2147483647l; }                                                    /* L to I */
void f5(void)  { unsigned u;       u = (unsigned)2147483647L; }                                               /* L to U */
void f6(void)  { unsigned long m;  m = (unsigned long)2147483647l; m = (unsigned long)-1L; }                  /* L to M */
void f7(void)  { unsigned char c;  c = (unsigned char)256ul;       c = (unsigned char)(unsigned long)-1; }    /* M to C */
void f8(void)  { unsigned short s; s = (unsigned short)65535Ul;    s = (unsigned short)65536uL; }             /* M to S */
void f9(void)  { int i1; static int i2 = (int)2147483648ul;        i1 = (int)2147483647UL; }                  /* M to I */
void f10(void) { unsigned u;       u = (unsigned)2147483647ul;     u = (unsigned)(unsigned long)-1; }         /* M to U */
void f11(void) { long l1; static long l2 = (long)2147483648ul;     l1 = (long)2147483647ul; }                 /* M to L */
void f12(void) { void *p;          p = (void *)1ul; }                                                         /* M to P */
void f13(void) { unsigned u;       u = (unsigned)(void *)1; }                                                 /* P to U */
void f14(void) { unsigned long m;  m = (unsigned long)(void *)1; }                                            /* P to M */
