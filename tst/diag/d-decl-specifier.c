const extern int a1;
const int extern a2;
const static int a3;
const int static a4;
void f1(void) { int auto a5;
                int register a6; }
extern extern int a7;
static static int a8;
void f2(void) { auto auto int a9;
                register register int a10; }
void f3(void) { int static static a11;
                int register register a12;
                auto auto int a13;
                static static int a14; }
const const int a15;
volatile volatile int a16;
const volatile int a17;
volatile const int a18;
const volatile const int a19;
typedef int x_t; signed x_t a20;
unsigned x_t a21;
short x_t a22;
long x_t a23;
double x_t a24;
typedef long double y_t; extern y_t a25; extern long double a25;
static a26; static int a26;
short char a27;
short double a28;
long float a29;
unsigned float a30;
signed double a31;
short short a32;
int int a33;
unsigned signed a34;
double float a35;
struct a36 { static int x; };
typedef const struct a37 t37; struct a37 { int x; }; t37 x37;
typedef const struct a38 t38; t38 x38; struct a38 { int x; };
typedef const struct a39 t39; t39 x39 = { 0 }; struct a39 { int x; };
void f40(void) {
typedef const struct a41 t41; struct a41 { int x; }; t41 x41;
typedef const struct a42 t42; t42 x42; struct a42 { int x; };
typedef const struct a43 t43; t43 x43 = { 0 }; struct a43 { int x; };
}
typedef volatile const struct a45 t45; struct a45 { int x; }; t45 x45;
typedef volatile const struct a46 t46; t46 x46; struct a46 { int x; };
typedef volatile const struct a47 t47; t47 x47 = { 0 }; struct a47 { int x; };
void f48(void) {
typedef volatile const struct a49 t49; struct a49 { int x; }; t49 x49;
typedef volatile const struct a50 t50; t50 x50; struct a50 { int x; };
typedef volatile const struct a51 t51; t51 x51 = { 0 }; struct a51 { int x; };
}
