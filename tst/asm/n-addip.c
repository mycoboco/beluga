int main(int argc, char **argv)
{
    int i = 0;

    i = (argc+1) + i;
    printf("%d\n", i);

    i = (argc+1U) + (unsigned)i;
    printf("%d\n", i);

    i = (int)((argv+1) + i);
    printf("%d\n", i);
}
