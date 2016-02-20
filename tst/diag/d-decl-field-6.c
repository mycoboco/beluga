struct t1 {
    const int x1: 3;
    int x2: 0;
    int :0;
    char x3: 1;
    signed char x4: 1;
    long x5: 1;
    double x6: 1;
    char x7: 0;
    char :0;
    int x8: 33;
    int x9: -2;
    int : -3;
    volatile int x10: 1;
    signed int x11: 1;
    unsigned int x12: 1;
};

void f(void)
{
    struct t1 x, y;

    x = y;
    x;
}
