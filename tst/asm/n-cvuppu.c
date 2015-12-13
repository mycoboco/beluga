int main(void)
{
    int *p = (void *)0xffffffff;
    unsigned u = 0xffffffff;

    printf("%u %p\n", (unsigned)p, (void *)u);
}
