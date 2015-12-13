int f(int i, int j)
{
    i >>= j;
    return i;
}

int main(void)
{
    printf("%d\n", f(-1, 10));
}
