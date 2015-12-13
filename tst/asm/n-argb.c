struct tag {
    int x[2];
    char c;
    char d;
    unsigned char e[3];
};

void f(struct tag x)
{
    printf("%d, %d, %d, %d, %d, %d, %d\n", x.x[0], x.x[1], x.c, x.d, x.e[0], x.e[1], x.e[2]);
}

int main(void)
{
    struct tag s = { 1, 2, 255, -128, -1, 255, -128 };
    f(s);
}
