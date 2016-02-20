/* --std=c90 -Wv */

void f(void) {
    int a;
    if (a) return;
    int b;
    if (b) return;
    int c;    /* no warning */
}

void g(void) {
    int a;
    ... [ [ if (c) return;
    = ^ ;
}

void h(void) {
    int a;
    ... [ [ int b;
    if (a) return;
    int c;
    ... [ [
}

void x(void) {
    return;
    int a;
}
