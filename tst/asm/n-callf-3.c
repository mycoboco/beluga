long double x(void)
{
    return 3.14;
}

int main(void)
{
    x();
    printf("%Lf\n", x());
}
