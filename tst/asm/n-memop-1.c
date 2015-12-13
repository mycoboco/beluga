int main(void)
{
    short s = 0x7ffe;
    int i = 0x7ffffffe;
    unsigned char uc = 254;

    s += 2;
    i += 2;
    uc += 2;
    printf("%d, %d, %d\n", s, i, uc);

    s -= 2;
    i -= 2;
    uc -= 2;
    printf("%d, %d, %d\n", s, i, uc);
}
