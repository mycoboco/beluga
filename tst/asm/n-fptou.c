int main(void)
{
    double f = 3.14;
    printf("%u\n", (unsigned)f);

    f = 0x7fffffff;
    printf("%u, %u\n", 0x7fffffff, (unsigned)f);

    f = 0x80000000;
    printf("%u, %u\n", 0x80000000, (unsigned)f);

    f = 0xffffffff;
    printf("%u, %u\n", 0xffffffff, (unsigned)f);

    f = 0xffffffff;
    f += 1;
    printf("%u\n", (unsigned)f);
}
