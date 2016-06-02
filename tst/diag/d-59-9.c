foo1_t x1 = 0;
foo2_t x2 = { 0 };
foo3_t x3 = { { 0, }, 1 };

bar1_t *y1 = 0;
bar2_t *y2 = { { 0, }, 1 };

struct {
    int *p;
    bar3_t m;
} s3 = { 1, { 0, 1, 2 } };

union {
    int *p;
    bar4_t q;
} u3 = { 1, { 0, 1, 2 } };

void func(void)
{
    fred1_t x1 = no;
    fred2_t x2 = { 0 };
    fred3_t x3 = { { 0, }, 1 };

    bart1_t *y1 = x1;
    bart2_t *y2 = { { 0, }, 1 };

    struct {
        bart3_t *m;
    } s3 = { a, x1 };
}

void g(f1_t p1 = 0, f2_t p2 = { 1, 2 });

void h(f1_t p1 = 0, f2_t p2 = { 1, 2 }) {}

void i(p1, p2) f1_t p1 = 0; f2_t p2 = { 1, 2 }; {}
