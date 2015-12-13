void f1(a) int a[sizeof &f1]; {}
static void f2(a) int a[sizeof &f2]; {}
extern void f3(a) int a[sizeof &f3]; {}
auto void f4(a) int a[sizeof &f4]; {}
register void f5(a) int a[sizeof &f5]; {}
typedef void f6(a) int a[sizeof &f6]; {}

void g1();
void g1(a) int a[sizeof &g1]; {}
void g2();
static void g2(a) int a[sizeof &g2]; {}
void g3();
extern void g3(a) int a[sizeof &g3]; {}
void g4();
auto void g4(a) int a[sizeof &g4]; {}
void g5();
register void g5(a) int a[sizeof &g5]; {}
void g6();
typedef void g6(a) int a[sizeof &g6]; {}

void h1(int *);
void h1(a) int a[sizeof &h1]; {}
void h2(int *);
static void h2(a) int a[sizeof &h2]; {}
void h3(int *);
extern void h3(a) int a[sizeof &h3]; {}
void h4(int *);
auto void h4(a) int a[sizeof &h4]; {}
void h5(int *);
register void h5(a) int a[sizeof &h5]; {}
void h6(int *);
typedef void h6(a) int a[sizeof &h6]; {}

typedef int i1;
void i1(a) int a[sizeof i1]; {}
typedef int i2;
void i2(a) int a[sizeof &i2]; {}
