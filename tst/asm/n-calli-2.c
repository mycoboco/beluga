int x1(int x) { int i = 0xffffffff; return i; }
char x2(int x) { int i = 0xffff; return i; }
short x3(int x) { int i = 0xfffff; return i; }
unsigned x4(int x) { return 0xffffffff; }

int main(void)
{
    x1(0);
    x2(0);
    x3(0);
    x4(0);
    printf("%d, %d, %d, %u\n", x1(0), x2(0), x3(0), x4(0));
}
