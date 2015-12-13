const extern int a1;    /* warning */
const int extern a2;    /* warning */
const static int a3;    /* warning */
const int static a4;    /* warning */
void f1(void) { int auto a5;          /* warning */
                int register a6; }    /* warning */
extern extern int a7;    /* error */
static static int a8;    /* error */
void f2(void) { auto auto int a9;               /* error */
                register register int a10; }    /* error */
void f3(void) { int static static a11;        /* error */
                int register register a12;    /* error */
                auto auto int a13;            /* error */
                static static int a14; }      /* error */
const const int a15;                /* error */
volatile volatile int a16;          /* error */
const volatile int a17;
volatile const int a18;
const volatile const int a19;    /* error */
typedef int x_t; signed x_t a20;    /* error */
unsigned x_t a21;                   /* error */
short x_t a22;                      /* error */
long x_t a23;                       /* error */
double x_t a24;                     /* error */
typedef long double y_t; extern y_t a25; extern long double a25;
static a26; static int a26;    /* warning */
short char a27;         /* error */
short double a28;       /* error */
long float a29;         /* error */
unsigned float a30;     /* error */
signed double a31;      /* error */
short short a32;        /* error */
int int a33;            /* error */
unsigned signed a34;    /* error */
double float a35;       /* error */
struct a36 { static int x; };    /* error */
typedef const struct a37 t37; struct a37 { int x; }; t37 x37;
typedef const struct a38 t38; t38 x38; struct a38 { int x; };
typedef const struct a39 t39; t39 x39 = { 0 }; struct a39 { int x; };    /* error */
void f40(void) {
typedef const struct a41 t41; struct a41 { int x; }; t41 x41;
typedef const struct a42 t42; t42 x42; struct a42 { int x; };            /* error */
typedef const struct a43 t43; t43 x43 = { 0 }; struct a43 { int x; };    /* error */
}
typedef volatile const struct a45 t45; struct a45 { int x; }; t45 x45;
typedef volatile const struct a46 t46; t46 x46; struct a46 { int x; };
typedef volatile const struct a47 t47; t47 x47 = { 0 }; struct a47 { int x; };    /* error */
void f48(void) {
typedef volatile const struct a49 t49; struct a49 { int x; }; t49 x49;
typedef volatile const struct a50 t50; t50 x50; struct a50 { int x; };            /* error */
typedef volatile const struct a51 t51; t51 x51 = { 0 }; struct a51 { int x; };    /* error */
}
