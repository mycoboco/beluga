void f1(void) { int i; i = !i; i = !!i; }
void f2(void) { char c; c = !c; c = !!c; }
void f3(void) { int *p; int i; i = !p; i = !!p;
                p = !p; }
void f4(void) { double x; x = !x; x = !!x; }
void f5(void) { struct { int x; } x; int i; i = !x; i = !!x; }
