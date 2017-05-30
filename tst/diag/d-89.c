struct { int x; } f(void), n[5];

void g(void)
{
    n[1] = f();
}
