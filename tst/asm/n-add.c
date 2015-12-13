int g;

int main(void)
{
    int i = 1;

    printf("%x\n", (unsigned)((int *)0 + i));
    printf("%x\n", (unsigned)(&g + i));
}
