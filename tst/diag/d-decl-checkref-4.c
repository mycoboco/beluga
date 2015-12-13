void f(void)
{
    void x1 = 0;
    const void x2 = { 0 };
    struct { int m; } x3 = 0;        /* x3 */
    struct { int m; } x4 = { 0 };    /* x4 */
}

void f2(void)
{
    int i1, i2;    /* i1 */
    struct { int m; } x;

    i1 = (x)? 1: 0;
    i2 = x || 1;
}

void f3(void)
{
    struct { int x; } x1, g();    /* x1 */
    int x2, h();                  /* x2 */

    x1 = g();
    x2 = h();
}

static struct foo { int x; } ws;
struct foo w, wf();
void f4(void)
{
    w = wf();
    ws = wf();
}
