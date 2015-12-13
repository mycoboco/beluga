/* -Wv */

/* bit-field type */
typedef int myint;

void f(void)
{
    struct {
        myint b:2;
    } s1;

    s1.b = (void *)0;
    s1.b = s1.b;
}

/* incomplete type */
typedef struct tag mys;
typedef mys mys2;

void f2(void)
{
    mys *x1;
    mys2 *x2;
    struct tag *x3;

    sizeof(*x1);
    sizeof(*x2);
    sizeof(*x3);
}

struct tag { int m; };

void f3(void)
{
    mys *x1;
    mys2 *x2;
    struct tag *x3;

    sizeof(*x1);
    sizeof(*x2);
    sizeof(*x3);
}

/* return type */
typedef void myvoid;

myint f4(void) {}
myvoid f5(void) {}
const myvoid f6(void) {}
f7(void) {}
