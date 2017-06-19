void f(void)
{
    long long ll1 = -(-0x7fffffffffffffff - 1);
    long long ll2 = -(-0x7fffffffffffffff);
    unsigned long long ull = -(unsigned long long)(-0x7fffffffffffffff - 1);
}
