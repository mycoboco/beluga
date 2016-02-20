static int z1;
static int z2 = 0;

void f(void)
{
    union { int x; } x, x1 = 0;    /* x1 suppressed */
    int x2 = x;                    /* x2 suppressed */
}

void g(void)
{
    int x;
    static int x3;
    static int x4 = 0;
    int x6;
    int x7 = 0;                    /* x7 */
    int x8 = x;                    /* x8 */
    int x10, x9;                   /* x10 */

    x9 = x;
    x10 = x9;
}
