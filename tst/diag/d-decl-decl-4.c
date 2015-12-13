void f1(a) int; {}
void f2(a) int; double a; {}
void f3(a) struct { int x; }; {}
void f4(a) enum { A }; {}
void f5(a) enum t5 { B }; {}
void f6(a) struct t6 { int x; }; {}
void f7(a) typedef int; {}

void g1(a) struct s11 { struct s12 { int x; }; }; {}
void g2(a) union { union { int x; }; }; {}
void g3(a) union u31 { enum u32 { C }; }; {}

void h1(int) {}
void h2(struct { int x; }) {}
void h3(enum { D }) {}
void h4(enum x4 { E }) {}
void h5(union x5 { int x; }) {}
void h6(struct x61 { struct x62 { int x; }; }) {}
void h7(union { union { int x; }; }) {}
void h8(union x81 { enum x82 { F }; }) {}
