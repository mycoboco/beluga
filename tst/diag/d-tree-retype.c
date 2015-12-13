extern void e;
int i;

void f(void)
{
    struct { int x: 10; } x;

    x = e;
    x.x = e;
    x = x.x = e;
    x.x = x;
    e(0);
    e(e);
    e();
    i();
    i(i);
}
