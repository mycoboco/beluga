static int x;
static int xx;

void f(void)
{
    struct {
        int *x;
    } y = { &x };    /* warning */

    struct {
        int x;
    } z = { xx };    /* warning */

    x = 0;
    xx = 0;
}
