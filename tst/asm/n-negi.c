int main(void)
{
    char c = 0;
    short s = 0;
    int i = 0x7ffffffe;
    unsigned u = 0xfffffffe;

    c = -(c-1);
    s = -(s-1);
    i = -(i+1);
    u = -(u+1);

    printf("%d %d %d %u\n", c, s, i, u);
}
