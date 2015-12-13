int main(void)
{
    short s = 0x1234;
    int i = 0x12345678;
    unsigned u = 0x12345678;
    unsigned char uc = 0x12;

    s |= 0xf;
    i |= 0xff;
    u |= 0xffff;
    uc |= 0xf;
    printf("%x, %x, %x, %x\n", (unsigned)s, u, (unsigned)i, (unsigned)uc);
}
