int f1(void) { int *i; char c; double d; i = (f1())? c: d; }    /* error - arithmetics */
int f2(void) { int i; int (*p1)(void); int (*p2)(); i = (f2())? p1: p2; }    /* error - compatible */
int f3(void) { int i; struct tag { int x; } x1, x2; i = (f3())? x1: x2; }    /* error - compatible */
int f4(void) { int i; int *p; i = (f4())? p: 0; i = (f4())? p: (void *)0; }    /* error - pointer, NPC */
int f5(void) { int i; int *p; i = (f5())? 0: p; i = (f5())? (void *)0: p; }    /* error - NPC, pointer */
int f6(void) { int i; int *p; void *pv; i = (f6())? p: pv; i = (f6())? pv: p; }    /* error - pointer, void * */
int f7(void) { int i; int (*p1)[10]; int (*p2)[]; i = (f7())? p1: p2; }    /* error - composite */
int f8(void) { int i; struct t1 x1; struct t2 x2; i = (f8())? x1: x2; }    /* error */
int f9(void) { int i; const int *p1; volatile int *p2; i = (f9())? p1: p2; }    /* error - qualification */
