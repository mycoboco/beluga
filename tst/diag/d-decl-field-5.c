/* -Wv --plain-int-field=unsigned */

struct {
    int x:2;                   /* warning */
    signed int y:2;
    unsigned int z:2;
    const int a:1;             /* warning */
    volatile int b:1;          /* warning */
    const volatile int c:1;    /* warning */
} x;

void f(void)
{
    x.x = -3;
    x.x = 2;
    x.x = -2;
    x.y = -3;    /* warning */
    x.y = 2;     /* warning */
    x.y = -2;
    x.z = -4;

    x.a = 1;    /* error */
    x.b = 1;
    x.c = 1;    /* error */
}
