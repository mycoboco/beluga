void f1(struct { int x; } x);
void f2(const union { int x; } x);
void f3(register volatile struct { int x; } x);
void f4(static union { int x; } x);
void f5(struct { int x; } x, struct { int y; } y);
void f6(struct { int x; } x) {}
void f7(const union { int x; } x) {}
void f8(register volatile struct { int x; } x) {}
void f9(static union { int x; } x) {}
void f10(struct { int x; } x, union { int y; } y) {}

void f11(x) struct { int x; } x; {}
void f12(x) const union { int x; } x; {}
void f13(x) register volatile struct { int x; } x; {}
void f14(x) static union { int x; } x; {}
void f15(x, y) struct { int x; } x, y; {}
void f16(x, y) struct { int x; } x; struct { int x; } y; {}

void f17(struct { struct { int x; } x; } x) {}
void f18(struct { struct { int x; } x; } x);
void f19(x) struct { struct { int x; } x; } x; {}
void f20(struct { int x; } x()) {}
void f21(struct { int x; } x());
void f22(x) struct { int x; } x(); {}
void f23(void x(struct { int x; }), struct { int x; } y) {}
void f24(void x(struct { int x; }), struct { int x; } y);
void f25(x, y) void x(struct { int x; }); struct { int x; } y; {}

struct { void (*f)(struct { int x; }); } x26;
struct { void (*f)(union { struct { int x; } x; } x); } x27;
void f28(struct { void (*f)(struct { int x; }); } x) {}
void f29(union { void (*f)(union { int x; }); } x);
void f30(x) struct { void (*f)(union { int x; }); } x; {}
