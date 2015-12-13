void f()
{
    struct { int x; char *p; } x1 = { 1, "xxx", 0, 3, { 0, "xxx", 1.5 }, 1.5 };             /* error */
    struct { int x:10, y:5, z:32; char *p; } x2 = { 0, 1, 2, "xxx", { 0, "xxx" }, 1.5 };    /* error */
    struct { int x:10, y:5, z:32; } x3 = { 0, 1, 2, 3, "xxx" };                             /* error */
    struct { char *p; int x:10, y:5, z:32; } x4 = { "xxx", 0, 1, 2, 3, 4 };                 /* error */
    struct { int x; char *p; } x5 = { 1.0, "xxx", if, while };                              /* error */

    f(x1, x2, x3, x4, x5);
}

void g(void)
{
    union { int x; char *p; } x1 = { 1, "xxx", 0, };                     /* error */
    union { int x:10, y:5, z:32; char *p; } x2 = { 0, 1, 2, "xxx", };    /* error */
    union { int x:10, y:5, z:32; } x3 = { 0, 1, 2, 3, "xxx" };           /* error */
    union { char *p; int x:10, y:5, z:32; } x4 = { "xxx", 0, 1, };       /* error */
    union { int x; char *p; } x5 = { 1.0, if, while };                   /* error */
    union { int x; char *p; } x6 = 0, y, z;                              /* error */

    f(x1, x2, x3, x4, x5, x6, y, z);
}

union { int x; char *p; } x6 = 0, y, z;    /* error */

void h(void) {
    static struct {
        int x[1024*1024*511+1024*1023+1023];    /* INT_MAX - size = 3 */
        union { int x; } z;
    } x = { { 0, }, 1, 1, 1 };    /* error */

    f(x);
}
