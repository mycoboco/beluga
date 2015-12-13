void f1(int static);
void f2(int static extern);
void f3(volatile register int);
void f4(volatile register int register);
void f5(struct { int x; } register);
void f6(struct { int x; } register register);
int auto x7;
int auto signed register x8;
void extern f9(void); void static f9(void);
const void extern f10(void); void static const static f10(void);
void static f11(void); void f11(void);
void f12(void) { void f122(void); } void static f122(void);
void f13(void) { void f132(void); } void static static f132(void);
static void f14(void) { int f14; { void f14(void); } } void f14(void);
void f15(void) { auto void f15(void); }
void f16(void) { auto void static f16(void); }
void f17(void) { volatile register int x; }
void f18(void) { volatile register int register x; }
void f19(void) { struct { int x; } register x; }
void f20(void) { struct { int x; } register register x; }
void f21(void) { int register a[10]; }
void f22(void) { int register register a[10]; }
static void f23(void) { int f23; { void extern f23(void); } }
static void f24(void) { int f24; { void f24(void); } }
typedef enum { A };
enum tag { B } typedef;
enum { C } typedef x27;
enum { D } typedef typedef y28;
int typedef;
int typedef extern;
void typedef f31(void) {}
void typedef typedef f32(void) {}
struct { int static x; } z33;
struct { int static extern x; } z34;
void f35(typedef enum { A });
void f36(enum { A } typedef);
void f37(typedef enum { B }) { }
void f38(enum { B } typedef) { }
void f39(void) { enum { A } typedef;
                 enum { B } extern typedef; }
