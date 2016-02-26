/* -Wv --std=c90 */

struct t { int x; int a[1]; };

void f(void)
{
    struct t g(), x;

    &((x = g()).a);    /* non-lvalue */
    &(g().a);          /* non-lvalue */
}
