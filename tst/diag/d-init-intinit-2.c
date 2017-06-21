/* -Wv --std=c90 */

struct tag {
    signed m3:   3;
    signed m32: 32;
    unsigned n3:   3;
    unsigned n32: 32;

    signed   m64: 64;
    unsigned n64: 64;
} x = {
    99e100,
    2147483648.0,
    99e100,
    4294967296.0
}, y = {
    0x0fffffff,
    0x80000000,
    0x0fffffff,
    0xffffffff,
};

void f(void)
{
    x.m3 = 99e100;
    y.m3 = 0x0fffffff;
    x.m32 = 2147483648.0;
    y.m32 = 0x80000000;
    x.n3 = 99e100;
    y.n3 = 0x0fffffff;
    x.n32 = 4294967296.0;
    y.n32 = 0xffffffff;
}
