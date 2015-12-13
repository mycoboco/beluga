void f1(void)  { long double ld;  ld = (long double)3.141592F; }                                  /* F to X */
void f2(void)  { long double ld;  ld = (long double)3.141592; }                                   /* D to X */
void f3(void)  { float f1;  static float f2 = (float)3.40283e+38L;    f1 = (float)3.141592l; }    /* X to F */
void f4(void)  { double d1; static double d2 = (double)1.79770e+308l; d1 = (float)3.141592l; }    /* X to D */
void f5(void)  { int i1;  static int i2 = (int)2147483648.0l;   i1 = (int)3.141592l; }            /* X to I */
void f6(void)  { long l1; static long l2 = (long)2147483648.0L; l1 = (long)3.141592l; }           /* X to L */
void f7(void)  { int i;           i = (int)(char)127;    i = (int)(char)255; }                    /* C to I */
void f8(void)  { unsigned u;      u = (unsigned)(unsigned char)127;
                                  u = (unsigned)(unsigned char)-1; }                              /* C to U */
void f10(void) { long l;          l = (long)(char)127;   l = (long)(char)255; }                   /* C to L */
void f11(void) { unsigned long m; m = (unsigned long)(unsigned char)127;
                                  m = (unsigned long)(unsigned char)-1; }                         /* C to M */
void f13(void) { int i;           i = (int)(short)32767; i = (int)(short)32768; }                 /* S to I */
void f14(void) { unsigned u;      u = (unsigned)(unsigned short)32767;
                                  u = (unsigned)(unsigned short)-1; }                             /* S to U */
void f16(void) { long l;          l = (long)(short)32767; l = (long)(short)32768; }               /* S to L */
void f17(void) { unsigned long m; m = (unsigned long)(unsigned short)32767;
                                  m = (unsigned long)(unsigned short)-1; }                        /* S to M */
