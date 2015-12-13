void f1(void) { float f; double d; f = d; d = f; }
void f2(void) { float f; long double x; f = x; x = f; }
void f3(void) { double d; long double x; d = x; x = d; }
void f4(void) { long double x; int i; x = i; i = x; }
void f5(void) { long double x; long l; x = l; l = x; }
void f6(void) { float f; int i; f = i; i = f; }
void f7(void) { double d; int l; d = l; l = d; }
