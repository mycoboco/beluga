int main(void)
{
    unsigned char uc = 0xfe;
    unsigned short us = 0xfffe;
    unsigned u;

    u = uc + 1;
    printf("%x\n", u);
    u = us + 1;
    printf("%x\n", u);
}
