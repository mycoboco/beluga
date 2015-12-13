int main(void)
{
    int i = -1;
    unsigned u = -1;

    printf("%d\n", ((int)0x80000000) * i);
    printf("%d\n", 2147483647 * i);
    printf("%u\n", 0xffffffff * u);
    printf("%u\n", (unsigned)i * u);
}
