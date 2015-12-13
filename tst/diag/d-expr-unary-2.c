void f1(void) { double x; int i; i = (double)x; x = *(double *)&i; }
void f2(void) { int *p, i; p = (int *)0; p = (int *)1; p = 0; p = 1; p = (int *)i; }    /* warning */
void f3(void) { enum tag { X } x; double y; y = (enum tag)y; y = (double)x; }
void f4(void) { struct tag { int x; } x; x = (struct tag)x; }    /* error */
void f5(void) { struct tag { int x; } x; int y; (void)x; (void)y; }
void f6(void) { int x; x = (x=0, x); }
