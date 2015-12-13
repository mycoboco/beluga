static int x;
static int xx;

void f(void)
{
    char y[] = { x };    /* warning */
    char z[xx];          /* warning */
    x = 0;
    xx = 0;
}

static int z;

void g()
{
    g(z);
    z = 0;
}
