void f1(void) { type_t int; }
void f2(void) { type_t foo; }
void f3(void) { type_t 42; }
void f4(void) { type_t 3.14; }
void f5(void) { type_t 'x'; }
void f6(void) { type_t "foo"; }
void f7(void) { type_t [12]; }
void f8(void) { type_t1 (); }
void f9(void) { type_t2 (42); }
void f10(void) { type_t {}; }
void f11(void) { type_t .m; }
void f12(void) { type_t ->m; }
void f13(void) { type_t++; }
void f14(void) { type_t &; }
void f15(void) { type_t * foo; }
void f16(void) { type_t * 42; }
void f17(void) { type_t + 42; }
void f18(void) { type_t ~; }
void f19(void) { type_t / 42; }
void f20(void) { type_t == 42; }
void f21(void) { type_t ^ 42; }
void f22(void) { type_t ?; }
void f23(void) { type_t ? 1: 0; }
void f24(void) { type_t: ; }
void f25(void) { type_t; }
void f26(void) { type_t ... ; }
void f27(void) { type_t = 42; }
void f28(void) { type_t += 42; }
void f29(void) { type_t, foo; }
void f30(void) { type_t # foo; }
void f31(void) { type_t ## foo; }
