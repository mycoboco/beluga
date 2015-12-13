void f1(void) { int i1; double d1; i1 = d1; }
void f2(void) { struct tag { int x; } x2; x2 = x2; }
void f3(void) { struct tag x3; x3 = x3; }    /* error */
void f4(void) { struct tag1 x4; struct tag2 y4; x4 = y4; }    /* error */
void f5(void) { void *p5; int *pi5; pi5 = p5; p5 = pi5; }
void f6(void) { const void *p6; int *pi6; pi6 = p6; p6 = pi6; }    /* error */
void f7(void) { int (**p7)[10]; int (* volatile *q7)[]; p7 = q7; q7 = p7; }    /* error */
void f8(void) { enum { X } *p8; int *q8; p8 = q8; }    /* warning */
