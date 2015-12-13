int main(void)
{
    int i = 16;
    unsigned u = 16;

    i = (i+1) % 3;
    printf("%d\n", i);

    i = -16;
    i = (i-1) % 3;
    printf("%d\n", i);

    u = (u+1) % 3;
    printf("%u\n", u);

    u = -16;
    u = (u-1) % 3;
    printf("%d\n", u);
}
