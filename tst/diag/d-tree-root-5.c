struct foo { int x; } f, func();
struct bar { int x; } b;

void x(void)
{
    int a[10], (*pa)[10];

    (f > b) < f;
    &func();

    a, a, a, a, a,, a;           /* warning */
    pa[0], *pa, pa[0],,  *pa;    /* warning */

    f = func();
    func();
}
