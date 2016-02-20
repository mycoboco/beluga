void f1(a);
void f2(a, b);
void f3(a, a);
void f4(a) { }
void f5(a, b) double b; { a = (void *)0; b = (void *)0; }
void f6(a, b) double a, b; { a = b; }
void f7(a, b) char a, b, c; { c = a; }
void f8(void) { (void *)0;
                (void *)1; (int)(void *)0; }
void f10(void) { int *p = 0, *r = (void *)0,
                     *q = 1,
                     *s = (void *)1; }
