/* -Wv */

typedef int f_t();
struct tag;

struct foo {
    f_t ;
    int *[10];
    double * ;
    struct tag;
    struct ;
    int (void);
    struct tag [];
    struct {};
    struct { int []; };
    struct { int m[]; };
    struct { struct tag m; };

    int m1[];
    struct tag m2;
    f_t m3;
    int m4(void);
    struct tag m5[1];
};
