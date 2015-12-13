void f(int i)
{
    i <<= 20;
    printf("%x\n", (unsigned)i);

    i = 1;
    i <<= 35;
    printf("%x\n", (unsigned)i);

    i = 1;
    i = (i << 20) >> 20;
    printf("%x\n", (unsigned)i);

    i = 1;
    i = (i << 36) >> 36;
    printf("%x\n", (unsigned)i);
}

int main(void)
{
    f(1);
}
