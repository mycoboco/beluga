int main(int argc, char **argv)
{
    int i = 1;
    unsigned u;

    i = (argc+1) << i;
    printf("%d\n", i);

    i = (argc+1) << 1;
    printf("%d\n", i);

    i = (argc+1) << 2;
    printf("%d\n", i);

    i = (argc+1) << 3;
    printf("%d\n", i);

    i = (argc+1) << 4;
    printf("%d\n", i);

    i = (argc+1) << 32;
    printf("%d\n", i);

    i = (argc+1) << (i=32);
    printf("%d\n", i);

    i = -1;
    printf("%d\n", i << 2);

    i = 1;
    u = ((unsigned)argc+1) << i;
    printf("%u\n", u);

    u = ((unsigned)argc+1) << 1;
    printf("%u\n", u);

    u = ((unsigned)argc+1) << 2;
    printf("%u\n", u);

    u = ((unsigned)argc+1) << 3;
    printf("%u\n", u);

    u = ((unsigned)argc+1) << 4;
    printf("%u\n", u);

    u = ((unsigned)argc+1) << 32;
    printf("%u\n", u);

    u = ((unsigned)argc+1) << (i=32);
    printf("%u\n", u);

    u = -1;
    printf("%u\n", u << 2);
}
