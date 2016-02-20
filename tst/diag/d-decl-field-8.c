/* -Wv */

struct {
    int x;
    int (*f)(int);
    union {
        int x;
    };
} x;

struct {
    typedef int y;
    int x;
    union {
        int x;
    };
} y;

struct {
    int (*f)(typedef int p);
    int x;
    union {
        int x;
    };
} z;

struct {
    int (*f)(struct tag {
        int x;
        union {
            int x;
        };
    });
    int x;
    union {
        int x;
    };
} w;
