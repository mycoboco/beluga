void f(void) {
    static int h1, h2(void), h3(void);
    register volatile int a, b;
    register struct { int a; } c, d;
    register int e[10], g[10], h;
    typedef int arr[10];
    register arr i, j, *k;
}
