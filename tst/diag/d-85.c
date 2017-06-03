typedef struct { int i; } foo_t;
typedef foo_t bar_t;
foo_t x;
bar_t y;
const foo_t cx;
typedef struct { int i; } fred_t;
fred_t z;
typedef union { int i; } uni_t;
uni_t u;

void f(void)
{
    x++;

    x[z];
    x[y];
    x[0];

    x.z;
    x->z;
    x.0;

    !x;
    *x;

    x + z;
    x + y;
    x + 0;

    x >> z;
    x >> y;
    x >> 0;

    x == z;
    x == y;
    x == 0;
    cx == x;
    (void)0 == (void)0;
    (const void)0 == (volatile void)0;
    u == u;

    x ^ z;
    x ^ y;
    x ^ 0;

    x && z;
    x || y;
    x && 0;

    (1)? x: z;
    (1)? x: y;
    (1)? x: 0;
}
