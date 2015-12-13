unsigned char c[] = "Hello";
char *s[] = { "first", "second", "third" };

void x1(void) { puts("x1"); }
void x2(void) { puts("x2"); }

void (*af[])(void) = { x1, x2 };

int main(int argc, char **argv)
{
    int i = 4;
    int ai[] = { 0, 1, 2, 3, };
    int aa[][3] = { 0, 1, 2,
                    3, 4, };
    int *p = (int *)aa;

    printf("%c\n", c[i]);
    printf("%c\n", *c);

    printf("%c\n", s[0][0]);
    printf("%c\n", s[1][1]);
    printf("%c\n", s[2][2]);

    printf("%d\n", ai[2]);
    printf("%d\n", ai[4]);

    printf("%d\n", aa[0][1]);
    printf("%d\n", aa[1][2]);

    printf("%d %d\n", p[i], p[5]);
    i = 0;
    af[i]();
    i = 1;
    af[i]();

    puts(argv[0]);
    printf("%c\n", argv[0][i]);
}
