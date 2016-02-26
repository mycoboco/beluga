/* -Wv --std=c90 */

struct s {
    int a[1];
} *t;

void f(void)
{
    void *p;

    ((struct s *)p)->a[0] = 0;    /* lvalue */
    (0, t)->a[0] = 0;             /* lvalue */
}

void g(void)
{
    struct s x(), y;

    x().a[0] = 0;          /* non-lvalue */
    (y = x()).a[0] = 0;    /* non-lvalue */
}
