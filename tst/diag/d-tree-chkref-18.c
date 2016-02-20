static int x;

void f(void)
{
    struct {
        int x: 2;
    } y = { x };

    x = 0;
}
