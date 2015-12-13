/* -Wv --_verbose-experr */

extern void x;
struct t { int x; } w;
struct tag { int x; int y:1; double z[5]; struct t w; };
int z;
void (*fp)();

/* void to struct */
void f1(void) {
    z = ((struct tag)x).x;
    ((struct tag)x).x = 0;
    z = ((struct tag)(void)1).x;
    ((struct tag)(void)1).x = 0;
    z = ((struct tag)f1()).x;
    ((struct tag)f1()).x = 0;
    z = ((struct tag)(z? x: x)).x;
    ((struct tag)(z? x: x)).x = 0;
    z = ((struct tag)(z? (void)1: (void)1)).x;
    ((struct tag)(z? (void)1: (void)1)).x = 0;
    z = ((struct tag)(z? f1(): f1())).x;
    ((struct tag)(z? f1(): f1())).x = 0;

    z = ((struct tag)x).y;
    ((struct tag)x).y = 0;
    z = ((struct tag)(void)1).y;
    ((struct tag)(void)1).y = 0;
    z = ((struct tag)f1()).y;
    ((struct tag)f1()).y = 0;
    z = ((struct tag)(z? x: x)).y;
    ((struct tag)(z? x: x)).y = 0;
    z = ((struct tag)(z? (void)1: (void)1)).y;
    ((struct tag)(z? (void)1: (void)1)).y = 0;
    z = ((struct tag)(z? f1(): f1())).y;
    ((struct tag)(z? f1(): f1())).y = 0;

    z = ((struct tag)x).z[1];
    ((struct tag)x).z[1] = 0;
    z = ((struct tag)(void)1).z[1];
    ((struct tag)(void)1).z[1] = 0;
    z = ((struct tag)f1()).z[1];
    ((struct tag)f1()).z[1] = 0;
    z = ((struct tag)(z? x: x)).z[1];
    ((struct tag)(z? x: x)).z[1] = 0;
    z = ((struct tag)(z? (void)1: (void)1)).z[1];
    ((struct tag)(z? (void)1: (void)1)).z[1] = 0;
    z = ((struct tag)(z? f1(): f1())).z[1];
    ((struct tag)(z? f1(): f1())).z[1] = 0;

    w = ((struct tag)x).w;
    ((struct tag)x).w = w;
    w = ((struct tag)(void)1).w;
    ((struct tag)(void)1).w = w;
    w = ((struct tag)f1()).w;
    ((struct tag)f1()).w = w;
    w = ((struct tag)(z? x: x)).w;
    ((struct tag)(z? x: x)).w = w;
    w = ((struct tag)(z? (void)1: (void)1)).w;
    ((struct tag)(z? (void)1: (void)1)).w = w;
    w = ((struct tag)(z? f1(): f1())).w;
    ((struct tag)(z? f1(): f1())).w = w;
}

/* void to function */
void f2(void) {
    ((void ())x)(0, 1);
    ((void ())(void)1)(0, 1);
    ((void ())f2())(0, 1);
    fp = ((void ())x);
    fp = ((void ())(void)1);
    fp = ((void ())f2());
}

/* void to array */
void f3(void) {
    z = ((int [10])x)[0];
    ((int [10])x)[0] = 1;
    z = ((int [10])(void)1)[1];
    ((int [10])(void)1)[1] = 1;
    z = ((int [10])f3())[2];
    ((int [10])f3())[2] = 1;
}

/* void to pointer */
void f4(void) {
    z = ((double *)x)[0];
    ((double *)x)[0] = 1;
    z = ((double *)(void)1)[1];
    ((double *)(void)1)[1] = 1;
    z = ((double *)f4())[1];
    ((double *)f4())[1] = 1;
    (*((double *)x))++;
    (*((double *)(void)1))++;
    (*((double *)f4()))++;
}

/* void to integer */
void f5(void) {
    z = ((unsigned)x)+1;
    z = ~((unsigned)(void)1);
    z = ((unsigned)f5()) % 1;
    z = +((unsigned)x);
    z = +((unsigned)(void)1);
    z = +((unsigned)f5());
}

/* void to fp */
void f6(void) {
    z = ((double)x)+0.1;
    z = ((double)(void)0.1)+0.1;
    z = ((double)f6())+0.1;
    z = -((double)x);
    z = -((double)(void)0.1);
    z = -((double)f6());
}
