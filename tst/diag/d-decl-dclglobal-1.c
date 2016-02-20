int *;
double x2_t; typedef int x2_t; x2_t x2_t;    /* redeclaration of x2_t twice */
auto int x3;
register int x4;
enum { x5 }; int x5;    /* redeclaration of x5 */
int x6[]; int x6[10]; void f(void) { int (*p)[10], (*q)[11], (*r)[]; p = &x6; q = &x6; r = &x6; }
int x7 = 0; int x7 = 0;    /* redefinition of x7 */
int x8 = 0; int x8;
extern int x9; static int x9;      /* linkage error */
static int x10; int x10;           /* linkage error */
int x11; static int x11;           /* linkage error */
static int x12; extern int x12;
int x13; extern int x13;
void g1(void) { extern int x14_15[10]; }
void g2(void) { extern int x14_15[]; } extern int x14_15[11];
void h1(void) { extern int x16_17_18[]; }
void h2(void) { extern int x16_17_18[10]; }
void h3(void) { extern int x16_17_18[11]; }
int x19(void) = 0;
static int x20[];
