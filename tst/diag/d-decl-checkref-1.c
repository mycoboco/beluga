void f1(void) { int x; }                          /* not referenced */
void f2(void) { int x; sizeof(x); }
void f3(int x) { }                                /* not referenced */
void f4(int x) { x; }
void f5(void) { static int x; }                   /* not referenced */
void f6(void) { static int x; &x; }
static int x7;                                    /* not referenced */
static int x8; void f8(int x) { x = x8; }
static void f9(void) { }                          /* not referenced */
static void f10(void) { f10; }
int x11;
static void f12(void); void x12(void) { f12; }    /* not defined */
static void f13(void);
