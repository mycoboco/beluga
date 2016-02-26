/* -Wv --std=c90 */

struct t {
    int a[1];
};

void f(void)
{
    struct t g();

    g().a++;    /* non-lvalue */
}
