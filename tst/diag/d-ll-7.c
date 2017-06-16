/* -Wv */

struct tag {
    int x: 3;
    unsigned y: 3;
};

void f(void)
{
    struct tag s1 = { 0xffffffff00000001, { 0xffffffff00000001 } };
    struct tag s2 = { 0x7fffffff00000001, { 0x7fffffff00000001 } };
}
