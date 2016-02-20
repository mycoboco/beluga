void f1(void) { char c; c = ~c; }
void f2(void) { double x; x = ~x; }
void f3(void) { struct t x; x = ~x; }

void f5(void) { char c; c = !c; }
void f6(void) { int *p; int i; i = !p; }
void f7(void) { int i; i = !f7; }
void f8(void) { struct t x; int i; i = !x; }
