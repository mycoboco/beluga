int x1(void) { int i = 0xffffffff; return i; }
char x2(void) { int i = 0xffff; return i; }
short x3(void) { int i = 0xfffff; return i; }
unsigned x4(void) { return 0xffffffff; }

int main(void)
{
    x1();
    x2();
    x3();
    x4();
    printf("%d, %d, %d, %u\n", x1(), x2(), x3(), x4());
}
