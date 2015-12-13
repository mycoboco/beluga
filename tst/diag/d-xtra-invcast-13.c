/* -Wv --_verbose-experr */

struct tag { int x; };
void f(struct tag);
int z;

/* void to struct */
void f1(void)
{
    extern void x;

    f((struct tag)x);
    f((struct tag)(void)1);
    f((struct tag)f1());
    f((struct tag)(z? x: x));
    f((struct tag)(z? (void)1: (void)1));
    f((struct tag)(z? f1(): f1()));
}

/* fp to struct */
void f2(void)
{
    double x;
    extern double ff(void);

    f((struct tag)x);
    f((struct tag)1.0);
    f((struct tag)ff());
    f((struct tag)(x? x: x));
    f((struct tag)(x? 1.0: 1.0));
    f((struct tag)(x? ff(): ff()));
}

/* integer to struct */
void f3(void)
{
    int x;
    extern int fi(void);

    f((struct tag)x);
    f((struct tag)1);
    f((struct tag)fi());
    f((struct tag)(x? x: x));
    f((struct tag)(x? 1: 1));
    f((struct tag)(x? fi(): fi()));
}

/* pointer to struct */
void f4(void)
{
    int *x;
    extern void *fp(void);

    f((struct tag)x);
    f((struct tag)(void *)1);
    f((struct tag)fp());
    f((struct tag)(x? x: x));
    f((struct tag)(x? (void *)1: (void *)1));
    f((struct tag)(x? fp(): fp()));
}
