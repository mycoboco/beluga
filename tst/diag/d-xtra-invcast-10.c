/* -Wv --_verbose-experr */

int z;
struct t { int x; } w;
struct tag { int x; int y:1; double z[5]; struct t w; };
void *x, *g();
void (*fp)();

void f(void)
{
    /* pointer to fp */
    z = (float)x + 0.1;
    z = (float)x + (float)x;
    z = (float)x > (float)x;
    z = -(float)x;
    z = (float)g() + 0.1;
    z = (float)g() + (float)g();
    z = (float)g() > (float)g();
    z = -(float)g();
    z = (float)(void *)1 + 0.1;
    z = (float)(void *)1 + (float)(void *)1;
    z = (float)(void *)1 > (float)(void *)1;
    z = -(float)(void *)1;

    /* pointer to struct */
    z = ((struct tag)x).x;
    ((struct tag)x).x = 0;
    z = ((struct tag)(void *)1).x;
    ((struct tag)(void *)1).x = 0;
    z = ((struct tag)g()).x;
    ((struct tag)g()).x = 0;
    z = ((struct tag)(x? x: x)).x;
    ((struct tag)(x? x: x)).x = 0;
    z = ((struct tag)(x? (void *)1: (void *)1)).x;
    ((struct tag)(x? (void *)1: (void *)1)).x = 0;
    z = ((struct tag)(g()? g(): g())).x;
    ((struct tag)(g()? g(): g())).x = 0;

    z = ((struct tag)x).y;
    ((struct tag)x).y = 0;
    z = ((struct tag)(void *)1).y;
    ((struct tag)(void *)1).y = 0;
    z = ((struct tag)g()).y;
    ((struct tag)g()).y = 0;
    z = ((struct tag)(x? x: x)).y;
    ((struct tag)(x? x: x)).y = 0;
    z = ((struct tag)(x? (void *)1: (void *)1)).y;
    ((struct tag)(x? (void *)1: (void *)1)).y = 0;
    z = ((struct tag)(g()? g(): g())).y;
    ((struct tag)(g()? g(): g())).y = 0;

    z = ((struct tag)x).z[1];
    ((struct tag)x).z[1] = 0;
    z = ((struct tag)(void *)1).z[1];
    ((struct tag)(void *)1).z[1] = 0;
    z = ((struct tag)g()).z[1];
    ((struct tag)g()).z[1] = 0;
    z = ((struct tag)(x? x: x)).z[1];
    ((struct tag)(x? x: x)).z[1] = 0;
    z = ((struct tag)(x? (void *)1: (void *)1)).z[1];
    ((struct tag)(x? (void *)1: (void *)1)).z[1] = 0;
    z = ((struct tag)(g()? g(): g())).z[1];
    ((struct tag)(g()? g(): g())).z[1] = 0;

    w = ((struct tag)x).w;
    ((struct tag)x).w = w;
    w = ((struct tag)(void *)1).w;
    ((struct tag)(void *)1).w = w;
    w = ((struct tag)g()).w;
    ((struct tag)g()).w = w;
    w = ((struct tag)(x? x: x)).w;
    ((struct tag)(x? x: x)).w = w;
    w = ((struct tag)(x? (void *)1: (void *)1)).w;
    ((struct tag)(x? (void *)1: (void *)1)).w = w;
    w = ((struct tag)(g()? g(): g())).w;
    ((struct tag)(g()? g(): g())).w = w;

    /* pointer to function */
    ((void ())x)(0, 1);
    ((void ())g())(0, 1);
    ((void ())(void *)1)(0, 1);
    fp = ((void ())x);
    fp = ((void ())g());
    fp = ((void ())(void *)1);

    /* pointer to array */
    z = ((int [10])x)[0];
    ((int [10])x)[0] = 1;
    z = ((int [10])(void *)1)[1];
    ((int [10])(void *)1)[1] = 1;
    z = ((int [10])g())[2];
    ((int [10])g())[2] = 1;
}
