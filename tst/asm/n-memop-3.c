int main(void)
{
    short s = 0x0f0f;
    int i = 0x0f0f0f0f;
    unsigned u = 0xf0f0f0f0;
    unsigned char uc = 0xf0;

    s = ~s;
    i = ~i;
    u = ~u;
    uc = ~uc;
    printf("%hx, %x, %x, %x\n", (unsigned short)s, u, (unsigned)i, (unsigned)uc);
}
