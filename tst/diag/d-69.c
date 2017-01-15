struct {
    int m;
} xoo, *poo;

void f(void)
{
    xoo.foo = 0;
    poo->bar = 1;
    xoo->foo = 0;
    poo.bar = 1;

    xoo. ++ = 0;
    poo. = 1;
}
