struct {
    int x:1;
    int y:2;
    unsigned w:1;
    unsigned z:2;
} x;

void f(void)
{
    x.x = 3;     /* warning */
    x.x = 2;     /* warning */
    x.x = 1;     /* warning */
    x.x = 0;
    x.x = -1;
    x.x = -2;    /* warning */
    x.x = -3;    /* warning */

    x.y = 3;     /* warning */
    x.y = 2;     /* warning */
    x.y = 1;
    x.y = 0;
    x.y = -1;
    x.y = -2;
    x.y = -3;    /* warning */

    x.w = 3;
    x.w = 2;
    x.w = 1;
    x.w = 0;
    x.w = -1;
    x.w = -2;
    x.w = -3;

    x.z = 3;
    x.z = 2;
    x.z = 1;
    x.z = 0;
    x.z = -1;
    x.z = -2;
    x.z = -3;
}
