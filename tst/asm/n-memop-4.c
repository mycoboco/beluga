int main(void)
{
    short s = -1;
    int i = -1;
    unsigned u = -1;
    unsigned char uc = -1;

    s = -s;
    i = -i;
    u = -u;
    uc = -uc;
    printf("%d, %u, %d, %d\n", s, u, i, uc);
}
