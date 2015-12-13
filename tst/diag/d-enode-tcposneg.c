void f1(void) { char c; c = +c; }
void f2(void) { short s; s = -s; }
void f3(void) { char *p; p = +p; p = -p; }    /* error */
void f4(void) { struct t x; x = +x; x = -x; }    /* error */
