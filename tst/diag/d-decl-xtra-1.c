enum t1 { C1 };
enum t2 { C2 }; typedef int enum_t;
void f3(enum t1);  void f3(x) enum t2 x; { }
void f4(enum t1);  void f4(x) const enum t2 x; { }
void f5(enum t1);  void f5(x) enum t1 x; { }
void f6(enum t1);  void f6(x) const enum t1 x; { }
void f7(enum t1);  void f7(enum t2);
void f8(enum t1);  void f8(enum t2 x) { }
void f9(enum t1);  void f9(enum t1);
void f10(enum t1); void f10(const enum t1);
void f11(enum t1); void f11(enum t1 x) { }
void f12(enum t1); void f12(enum_t);
void f13(enum t1); void f13(const int);
enum t1 x14;       enum_t x14;
enum t1 x15[10];   enum_t x15[10];
enum t1 x16(void); enum_t x16(void);
enum t1 x17;       const enum_t x17;
const enum t1 x18; volatile enum_t x18;
const enum t1 x19; enum_t x19;
const enum t1 x20; const enum_t x20;
void f21_22(void (*)(enum t1));
void f21_22(x) void (*x)(enum t1); { }
void f23_24(void (*)(enum t1));
void f23_24(x) void (*x)(enum_t); { }
void f25_26(void (*)(enum t1));
void f25_26(void (*)(enum t2));
void f27_28(void (*)(enum_t));
void f27_28(x) void (*x)(enum t1); { }
const enum t1 x29; enum t1 x29;
