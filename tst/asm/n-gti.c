int main(void)
{
    int i = -1;
    unsigned u = -1;

    if (i <= (int)u) puts("i <= u"); else puts("i > u");
    if ((unsigned)i <= u) puts("i <= u"); else puts("i > u");
}
