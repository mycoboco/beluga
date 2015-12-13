int main(void)
{
    short s = -1;
    int i = -1;
    unsigned u = -1;
    unsigned char uc = -1;

    s >>= 2;
    i >>= 2;
    u >>= 2;
    uc >>= 2;
    printf("%d, %x, %d, %x\n", s, u, i, (unsigned)uc);

    s <<= 2;
    i <<= 2;
    u <<= 2;
    uc <<= 2;
    printf("%d, %x, %d, %x\n", s, u, i, (unsigned)uc);
}
