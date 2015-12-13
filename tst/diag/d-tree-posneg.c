void f1(void) { char c; unsigned char uc; c = +c; c = +uc; }
void f2(void) { struct { int x; } x; int i; i = +x; }    /* error */
void f3(void) { char c; unsigned char uc; c = -c; c = -uc; }
void f4(void) { int i; unsigned u; i = -i; i = -u; }
