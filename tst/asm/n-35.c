struct bar {
    struct foo {
        int foo;
    } *pfoo;
};

struct foo foo = { 1 }, foo2 = { 2 };
struct bar bar = { &foo2 };

static void f(int j)
{
    struct bar *p = &bar;
    struct foo *n = &foo;

    n[j] = p->pfoo[j];
}

int main(void)
{
    printf("%d\n", foo.foo);
    f(0);
    printf("%d\n", foo.foo);
}
