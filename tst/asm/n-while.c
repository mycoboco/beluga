int main(void)
{
    const char *s = "while test";
    char t[64], *p;

    p = t;
    while (*p++ = *s++)
        ;

    puts(t);
}
