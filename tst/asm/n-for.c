int main(void)
{
    int i;
    const char *s = "loop test";
    char t[64], *p;

    for (i = 1; i <= 100; i++)
        printf("%d\n", i);

    for (p = t; *p++ = *s++; )
        continue;
    puts(t);
}
