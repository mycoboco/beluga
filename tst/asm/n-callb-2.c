struct tag {
    char c;
    int i;
    char a[3];
};

struct tag x(int p)
{
    struct tag y = { 1, 2, 3, 4, 5 };
    y.c = p;

    return y;
}

int main(void)
{
    struct tag z = x(10);

    printf("%d %d %d %d %d\n", z.c, z.i, z.a[0], z.a[1], z.a[2]);
}
