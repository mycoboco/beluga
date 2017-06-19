void f(void)
{
    int i1 = -(-0x7fffffff - 1);
    int i2 = -(-0x7fffffff);
    unsigned u = -(unsigned)(-0x7fffffff - 1);
    long l1 = -(-0x7fffffff - 1);
    long l2 = -(-0x7fffffff);
    unsigned long ul = -(unsigned long)(-0x7fffffff - 1);
}
