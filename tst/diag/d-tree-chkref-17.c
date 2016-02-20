static int x;
static int xx;

void f(void)
{
    struct {
        int *x;
    } y = { &x };

    struct {
        int x;
    } z = { xx };

    x = 0;
    xx = 0;
}
