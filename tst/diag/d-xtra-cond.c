struct incomp *x, *y, *z;

void f(void)
{
    extern int foo;
    *x = (foo)? *y: *z;
}
