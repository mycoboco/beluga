void f1(void) { int i, *p; i = (f1)? 1: 0; i = (i)? 1: 0; i = (p)? 1: 0; }    /* warning */
void f2(void) { struct t x; int i; i = (x)? 1: 0; }    /* error */

void f4(void) { int i, *p; i = f1 && f1; i = i || i; i = p && p; }    /* warning */
void f5(void) { struct t x; int i; i = x || 1; }    /* error */

void f7(void) { int i, *p; for (; f1; ); for (; i; ); for (; p; ); }    /* warning */
void f8(void) { struct t x; for (; x; ); }    /* error */

void f10(void) { int i, *p; while (f1); while (i); while (p); }    /* warning */
void f11(void) { struct t x; while (x); }    /* error */

void f13(void) { int i, *p; do; while(f1); do; while(i); do; while(p); }    /* warning */
void f14(void) { struct t x; do; while(x); }    /* error */
