void g(int n)
{
    printf("%d\n", n);
}

void f(int n)
{
    g(n++);
    printf("%d\n", n);
}

int main(void)
{
    f(3);
}
