enum { X } x1; int x1; int x1;    /* warning */
unsigned char x2; unsigned char x2;
const int x3[3]; const int x3[]; int x3[3];    /* error */
int (*x4(void))[10]; int (*x4(void))[]; int (*x4(void))[10];
void x5(a) char a; { } void x5(); void x5(int);
void x6(a) char a; { } void x6(); void x6(double);    /* error */
void x7(int (*p[])(int)); void x7(int (*p[3])()); void x7(int (*p[3])(int));
