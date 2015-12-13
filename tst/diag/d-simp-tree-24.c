void f(void)
{
    struct {
        int x, y;
    } x;

    if (&x)
        ;
    if ("abc")
        ;
    if (!"abc")
        ;
    if (&(x.x))
        ;
    if (&(x.y))
        ;
}
