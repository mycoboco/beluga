static int x;
static int y;
static int z;

void f(void)
{
    int *p[] = { &x };
    int q = y;

    x = 0;
    y = 0;
    z = 0;
}

static int xx = 1;
static int yy = 1;

int *p = { &xx };
int q = yy;
