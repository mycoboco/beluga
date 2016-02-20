extern struct t x;
extern struct s y;

int f(void)
{
    int i;

    if (i)
        return 3.14;
    else if (i+1)
        return &i + 1;
    else
        return x;
}

struct t g(void)
{
    if (f())
        return x;
    else
        return y;
}

struct tag { int x; } h(void)
{
    struct tag y;

    if (f())
        return y;
    else
        return h();
}

const struct tag m(struct tag y)
{
    if (f())
        return m(y);
    return y;
}

struct foo { const int x; } n(struct foo y)
{
    if (f())
        return n(y);
    return y;
}
