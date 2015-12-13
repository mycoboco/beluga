void f1(void) { int i; struct t { int x:+1+2; int y:i+1; }; }    /* error */
void f2(void) { int i; enum { X = +3+10, Y = i+1 }; }    /* error */
void f3(void) { int i; int a1[+10-9]; int a2[i+1]; a1, a2; }    /* error */
void f4(void) { int f(void); int i; switch(f()) { case +3-3: ; case i+1: case 3.14: break; } }    /* error */
