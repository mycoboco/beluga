void f(void) { }
void f(void) { }            /* redefinition */
void g(void) { }
void g(int a) { }           /* redeclaration */
void h(int a) { }
void h(enum { A } a) { }    /* redefinition, warning */
int a = 0;
int a = 0;           /* redefinition */
int b = 0;
double b = 0;        /* redeclaration */
int c = 0;
enum { B } c = 0;    /* redefinition, warning */
void h1(int a, int a) { }       /* redeclaration */
void h2(int a, double a) { }    /* redeclaration */
void j1(a, a) int a; int a; { }       /* redeclaration */
void j2(a, a) int a; double a; { }    /* redeclaration */
void k1(void) { int a; int a; }       /* redeclaration */
void k2(void) { int a; double a; }    /* redeclaration */
