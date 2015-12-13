void f1(void) { int a[2], b[2] = a; a = b; }    /* error */
void f2_3(void) { struct tag { const int x; } x, y = x;
                                                 x = y; }    /* error */
void f4(void) { int x; switch(x + 1) { case 0: return; case 1: break; default: return; } }
void f5(void) { extern int f(void); int a[2]; switch(a[0]+f()) { default: return ; } }
