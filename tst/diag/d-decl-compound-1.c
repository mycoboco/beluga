void f1(void) { lab: f1(); goto lab; }
void f2(void) { f1(); int x; }            /* error */
