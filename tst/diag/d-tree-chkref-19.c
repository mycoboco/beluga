static int x;
static int xx;

void f(void)
{
    char y[] = { x };
    char z[xx];
    x = 0;
    xx = 0;
}

static int z;

void g()
{
    g(z);
    z = 0;
}
