/* -Wv --_verbose-experr */

extern unsigned x;
struct t { int x; } w;
struct tag { int x; int y:1; double z[5]; struct t w; };
extern int f(void);
int z;
void (*fp)();

/* integer to struct */
void f1(void) {
    z = ((struct tag)x).x;
    ((struct tag)x).x = 0;
    z = ((struct tag)1).x;
    ((struct tag)1).x = 0;
    z = ((struct tag)f()).x;
    ((struct tag)f()).x = 0;
    z = ((struct tag)(x? x: x)).x;
    ((struct tag)(x? x: x)).x = 0;
    z = ((struct tag)(x? 1: 1)).x;
    ((struct tag)(x? 1: 1)).x = 0;
    z = ((struct tag)(x? f(): f())).x;
    ((struct tag)(x? f(): f())).x = 0;

    z = ((struct tag)x).y;
    ((struct tag)x).y = 0;
    z = ((struct tag)1).y;
    ((struct tag)1).y = 0;
    z = ((struct tag)f()).y;
    ((struct tag)f()).y = 0;
    z = ((struct tag)(x? x: x)).y;
    ((struct tag)(x? x: x)).y = 0;
    z = ((struct tag)(x? 1: 1)).y;
    ((struct tag)(x? 1: 1)).y = 0;
    z = ((struct tag)(x? f(): f())).y;
    ((struct tag)(x? f(): f())).y = 0;

    z = ((struct tag)x).z[1];
    ((struct tag)x).z[1] = 0;
    z = ((struct tag)1).z[1];
    ((struct tag)1).z[1] = 0;
    z = ((struct tag)f()).z[1];
    ((struct tag)f()).z[1] = 0;
    z = ((struct tag)(x? x: x)).z[1];
    ((struct tag)(x? x: x)).z[1] = 0;
    z = ((struct tag)(x? 1: 1)).z[1];
    ((struct tag)(x? 1: 1)).z[1] = 0;
    z = ((struct tag)(x? f(): f())).z[1];
    ((struct tag)(x? f(): f())).z[1] = 0;

    w = ((struct tag)x).w;
    ((struct tag)x).w = w;
    w = ((struct tag)1).w;
    ((struct tag)1).w = w;
    w = ((struct tag)f()).w;
    ((struct tag)f()).w = w;
    w = ((struct tag)(x? x: x)).w;
    ((struct tag)(x? x: x)).w = w;
    w = ((struct tag)(x? 1: 1)).w;
    ((struct tag)(x? 1: 1)).w = w;
    w = ((struct tag)(x? f(): f())).w;
    ((struct tag)(x? f(): f())).w = w;
}

/* integer to function */
void f2(void) {
    ((void ())x)(0, 1);
    ((void ())1)(0, 1);
    ((void ())f())(0, 1);
    fp = ((void ())x);
    fp = ((void ())1);
    fp = ((void ())f());
}

/* integer to array */
void f3(void) {
    z = ((int [10])x)[0];
    ((int [10])x)[0] = 0;
    z = ((int [10])1)[1];
    ((int [10])1)[1] = 0;
    z = ((int [10])f())[2];
    ((int [10])f())[2] = 0;
}
