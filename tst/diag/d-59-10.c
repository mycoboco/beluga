extern foo1_t x1;
static foo1_t x1;

void f(void)
{
    extern foo2_t x2;
}

static foo2_t x2;

static foo3_t x3;
extern foo3_t x3;

void g(void)
{
    int x3;
    {
        extern foo3_t x3;
    }
}
