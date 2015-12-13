struct tag {
    char c;
    int i;
    char a[3];
} x = { 1, 2, 3, 4, 5 };

int main(void)
{
    struct tag s;

    s = x;
    printf("%d %d %d %d %d\n", s.c, s.i, s.a[0], s.a[1], s.a[2]);
}
