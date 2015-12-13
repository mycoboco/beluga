/* -Wv */

struct x1 {
    int a;
    struct x1 m;    /* error */
};

struct x2 {
    int a;
    struct y1 {
        int b;
        struct x2 m;    /* error */
    } x;
};

struct x3 {
    int a;
    struct y2 m;    /* error */
    struct y2 {
        int b;
    } n;
};

struct x4 {
    int a;
    struct y3 m;    /* error */
    struct y3 {
        int b;
        struct x4 n;    /* error */
    } o;
};
