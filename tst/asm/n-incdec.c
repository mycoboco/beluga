int main(void)
{
    short s = 0x7fff;
    int i = 0x7fffffff;
    unsigned char uc = 255;

    s++;
    i++;
    uc++;
    printf("%d, %d, %d\n", s, i, uc);

    s--;
    i--;
    uc--;
    printf("%d, %d, %d\n", s, i, uc);
}
