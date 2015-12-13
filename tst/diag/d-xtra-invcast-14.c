/* -Wv --_verbose-experr */

struct tag { int x; };
int z;

/* void to struct */
void f1(void)
{
    extern void x;

    z = (struct tag)x > z;
    z = (struct tag)(void)1 > z;
    z = (struct tag)f1() > z;
    z = (struct tag)(z? x: x) > z;
    z = (struct tag)(z? (void)1: (void)1) > z;
    z = (struct tag)(z? f1(): f1()) > z;
}

/* fp to struct */
void f2(void)
{
    double x;
    extern double ff(void);

    z = (struct tag)x > z;
    z = (struct tag)1.0 > z;
    z = (struct tag)ff() > z;
    z = (struct tag)(x? x: x) > z;
    z = (struct tag)(x? 1.0: 1.0) > z;
    z = (struct tag)(x? ff(): ff()) > z;
}

/* integer to struct */
void f3(void)
{
    int x;
    extern int fi(void);

    z = (struct tag)x > z;
    z = (struct tag)1 > z;
    z = (struct tag)fi() > z;
    z = (struct tag)(x? x: x) > z;
    z = (struct tag)(x? 1: 1) > z;
    z = (struct tag)(x? fi(): fi()) > z;
}

/* pointer to struct */
void f4(void)
{
    int *x;
    extern void *fp(void);

    z = (struct tag)x > z;
    z = (struct tag)(void *)1 > z;
    z = (struct tag)fp() > z;
    z = (struct tag)(x? x: x) > z;
    z = (struct tag)(x? (void *)1: (void *)1) > z;
    z = (struct tag)(x? fp(): fp()) > z;
}
