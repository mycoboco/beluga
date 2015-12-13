int a[2];

int *g(void)
{
    return a;
}

void f(void)
{
    printf("%d\n", g()[1]++);
}

int main(void)
{
    f();
    printf("%d\n", a[1]);
}
