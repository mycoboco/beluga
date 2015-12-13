typedef struct foo foo_t;

void g(const foo_t *);
struct foo { int i; };

foo_t *q;

void f(void)
{
    g(q);
}
