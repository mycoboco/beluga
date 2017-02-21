/* -WWv */

struct {
    union {
        int m;
    };
} x1;

struct {
    union tag {
        int m;
    };
} x2;

union foo {
    int m;
};

struct {
    union foo;
} x3;
