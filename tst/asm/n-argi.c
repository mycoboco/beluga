void f(char c, unsigned char uc, short s, int i, unsigned u, long l)
{
    printf("%d, %d, %d, %d, %u, %ld\n", c, uc, s, i, u, l);
}

int main(void)
{
    f(-1, 255, 32767, -32769, 0xfffff, 32768);
    f(-129, 256, 32768, 0xffffffff, 0xffffffffU+1, 0xffffffff);
}
