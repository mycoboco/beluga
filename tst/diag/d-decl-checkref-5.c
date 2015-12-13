static int z1;
static int z2 = 0;

void f(void)
{
    union { int x; } x, x1 = 0;    /* warning */
    int x2 = x;                    /* warning */
    static int x3;
    static int x4 = 0;
    static int x5 = x;
    int x6;
    int x7 = 0;                    /* warning */
    int x8 = x;                    /* warning */
    int x10, x9;                   /* x10 */

    x9 = x;
    x10 = x9;
}
