/* -Wv */

struct {
    int x;
    int (*f)(int);
    union {
        int x;    /* error */
    };
} x;

struct {
    typedef int y;
    int x;
    union {
        int x;    /* error */
    };
} y;

struct {
    int (*f)(typedef int p);
    int x;
    union {
        int x;    /* error */
    };
} z;

struct {
    int (*f)(struct tag {
        int x;
        union {
            int x;    /* error */
        };
    });
    int x;
    union {
        int x;
    };
} w;
