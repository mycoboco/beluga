int e1 = { { { { , } } } };
int e2 = { { { { , , } } } };       /* extrabrace */
int e3 = { { { { 0, } } } };
int e4 = { { { { 0, , } } } };      /* extrabrace */
int e5 = { { { { 0, , , } } } };    /* extrabrace */

void x(int p1, int p2, )    /* parameter-1 */
{
    int a1[] = { , };
    int a2[] = { , , };             /* arrayinit */
    int a3[] = { 0, 1, 2, };
    int a4[] = { 0, 1, 2, , };      /* arrayinit */
    int a5[] = { 0, 1, 2, , 3 };    /* arrayinit */
    int a6[] = { 0, 1, 2, , , };    /* arrayinit */
    char c1[] = { , };
    char c2[] = { , , };             /* carrayinit */
    char c3[] = { 0, 1, 2, };
    char c4[] = { 0, 1, 2, , };      /* carrayinit */
    char c5[] = { 0, 1, 2, , 3 };    /* carrayinit */
    char c6[] = { 0, 1, 2, , , };    /* carrayinit */
    int i1 = { , };
    int i2 = { , , };       /* dcllocal */
    int i3 = { 0, };
    int i4 = { 0, , };      /* dcllocal */
    int i5 = { 0, , , };    /* dcllocal */

    long ,;
    long , , ;
    double d1, d2, , d3;    /* decl */
    unsigned u1, u2, , ;    /* decl */
    float d4, d5,

    struct tag {
        int ,;           /* field */
        int , , ;        /* field */
        int a, b,;       /* field */
        int c, d,
        int e, , , f;    /* field */
        int g, , ;       /* field */
    } s1 = { 1, 2, , , };                /* structinit */
    struct tag s2 = { , };
    struct tag s3 = { , , };             /* structinit */
    struct tag s4 = { 1, 2, , , 3, };    /* structinit */

    struct field {
        int a:1, b:2, c:3;
    } fd1 = { , },
      fd2 = { , , },       /* fieldinit */
      fd3 = { 1, },
      fd4 = { 1, , },      /* fieldinit */
      fd5 = { 1, , , };    /* fieldinit */

    struct fm {
        int a:1, b:2, c:3;
        int m;
    } fm1 = { 1, , , },             /* fieldinit */
      fm2 = { 1, 1, , , 1, },       /* fieldinit */
      fm3 = { 1, 1, 1, 1, , , };    /* structinit */

    struct incomp in1 = { , },
                  in2 = { , , },             /* skipinit-1 */
                  in3 = { , , , },           /* skipinit-1 */
                  in4 = { 1, },
                  in5 = { 1, , },            /* skipinit-1 */
                  in6 = { { 1, }, 1, , },    /* skipinit-1 */
                  in7 = { { 1, }, , , };     /* skipinit-2 */

    enum { , };
    enum { A = 0, B, C, };
    enum { D, E,, };          /* enumdcl */
    enum { F,,, };            /* enumdcl */
    enum { G, , H };          /* enumdcl */

    extern void f1(,);
    extern void f2(, ,);
    extern void f3(int, float,);          /* parameter-1 */
    extern void f4(int, float, , int);    /* parameter-1 */

    g(,);               /* tree_pcall */
    g(, ,);             /* tree_pcall */
    g(1, 2, 3,);        /* tree_pcall */
    g(1, 2, 3, ,);      /* tree_pcall */
    g(1, 2, 3, , 4);    /* tree_pcall */

    ,;
    , ,;
    g, ;       /* expr_expr */
    g, , ;     /* expr_expr */
    g, , g;    /* expr_expr */
}

void y1(,) {}
void y2(, ,) {}
void y3(int p1, int p2,) {}               /* parameter-1 */
void y4(int p1, int p2, ,) {}             /* parameter-1 */
void y5(int p1, int p2, , , int p3) {}    /* parameter-1 */

void z1(a, b,) {}        /* parameter-2 */
void z2(a, b, ,) {}      /* parameter-2 */
void z3(a, b, , c) {}    /* parameter-2 */
