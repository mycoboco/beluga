void f1(void)  { long double ld;   ld = (long double)3; }                                                 /* I to X */
void f2(void)  { char c1; static char c2 = (char)128, c3 = (char)-129;         c1 = (char)127; }          /* I to C */
void f3(void)  { short s1; static short s2 = (short)32768, s3 = (short)-32769; s1 = (short)32767; }       /* I to S */
void f4(void)  { unsigned u;       u = (unsigned)2147483647;       u = (unsigned)-1; }                    /* I to U */
void f5(void)  { long l;           l = (long)2147483647;           l = (long)(-2147483647-1); }           /* I to L */
void f6(void)  { unsigned long m;  m = (unsigned long)2147483647;  m = (unsigned long)-1; }               /* I to M */
void f7(void)  { unsigned char c;  c = (unsigned char)255U;        c = (unsigned char)(unsigned)-1; }     /* U to C */
void f8(void)  { unsigned short s; s = (unsigned short)65535u;     s = (unsigned short)(unsigned)-1; }    /* U to S */
void f9(void)  { int i1; static int i2 = (int)(unsigned)-1;        i1 = (int)2147483647u; }               /* U to I */
void f10(void) { long l1; static long l2 = (long)(unsigned)-1;     l1 = (long)2147483647u; }              /* U to L */
void f11(void) { unsigned long m;  m = (unsigned long)4294967295U; m = (unsigned long)(unsigned)-1; }     /* U to M */
void f12(void) { void *p;          p = (void *)1u; }                                                      /* U to P */
