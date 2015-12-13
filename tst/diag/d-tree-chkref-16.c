static int x;
static int y;
static int z;    /* warning */

void f(void)
{
    int *p[] = { &x };    /* warning */
    int q = y;            /* warning */

    x = 0;
    y = 0;
    z = 0;
}

static int xx = 1;
static int yy = 1;

int *p = { &xx };
int q = yy;
