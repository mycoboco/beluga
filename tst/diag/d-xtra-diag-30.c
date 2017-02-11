void x1 = 0;
void x2;
struct tag x3 = { 0, };
struct tag x4;

void f(void)
{
    void z1 = 0;
    const void z2;
    struct tag y1 = { 0, };
    struct tag y2 = y1;
    struct tag y3;
}
