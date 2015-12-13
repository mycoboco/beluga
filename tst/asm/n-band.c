int main(void)
{
    int i = 0x12345678;
    unsigned u = 0x12345678;

    printf("%x\n", (unsigned)(i & 0xffff));
    printf("%x\n", u & 0xffff);
}
