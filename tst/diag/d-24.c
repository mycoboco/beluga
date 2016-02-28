typedef pi1;
typedef int pi2;
typedef signed int si;
typedef unsigned int ui;

struct t {
    pi1 x1:        1;    /* plain */
    pi2 x2:        1;    /* plain */
    si  x3:        1;
    ui  x4:        1;
    int x5:        1;    /* plain */
    signed x6:     1;
    signed int x7: 1;
    unsigned x8:   1;

    const pi1 x9:         1;    /* plain */
    const pi2 x10:        1;    /* plain */
    const si  x11:        1;
    const ui  x12:        1;
    const int x13:        1;    /* plain */
    const signed x14:     1;
    const signed int x15: 1;
    const unsigned x16:   1;
};
