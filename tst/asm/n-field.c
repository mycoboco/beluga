struct {
    char c[3];
    struct {
        unsigned a:1;
        signed b:1;
        unsigned c:2;
        signed d:2;
    } x;
    int e;
} s = { 1, 2, 3,
        3, 3, 3, 3,
        -1 };

int main(void)
{
    printf("%d %d %d %d %d %d %d %d\n", s.c[0], s.c[1], s.c[2],
                                        s.x.a, s.x.b, s.x.c, s.x.d,
                                        s.e);

    s.x.a = 0;
    s.x.b = 1;
    s.x.c++;
    s.x.d++;

    printf("%d %d %d %d %d %d %d %d\n", s.c[0], s.c[1], s.c[2],
                                        s.x.a, s.x.b, s.x.c, s.x.d,
                                        s.e);

    s.x.c++;
    s.x.d++;

    printf("%d %d %d %d %d %d %d %d\n", s.c[0], s.c[1], s.c[2],
                                        s.x.a, s.x.b, s.x.c, s.x.d,
                                        s.e);

    s.x.c++;
    s.x.d++;

    printf("%d %d %d %d %d %d %d %d\n", s.c[0], s.c[1], s.c[2],
                                        s.x.a, s.x.b, s.x.c, s.x.d,
                                        s.e);
}
