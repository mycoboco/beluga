struct {
    int x:2;
    signed int y:2;
    unsigned int z:2;
    const int a:1;
    volatile int b:1;
    const volatile int c:1;
} x;

void f(void)
{
    x.x = -3;
    x.x = 2;
    x.x = -2;
    x.y = -3;
    x.y = 2;
    x.y = -2;
    x.z = -4;

    x.a = 1;
    x.b = 1;
    x.c = 1;
}
