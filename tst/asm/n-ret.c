char *x(int i)
{
    static char s[] = "hello";

    return &s[i];
}

char *(*y(void))(int)
{
    return x;
}

int main(void)
{
    printf("%c\n", *x(1));
    printf("%c\n", *y()(0));
}
