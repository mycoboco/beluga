/* -Wv */

struct x1 {
    int a;
    struct x1 m;
};

struct x2 {
    int a;
    struct y1 {
        int b;
        struct x2 m;
    } x;
};

struct x3 {
    int a;
    struct y2 m;
    struct y2 {
        int b;
    } n;
};

struct x4 {
    int a;
    struct y3 m;
    struct y3 {
        int b;
        struct x4 n;
    } o;
};
