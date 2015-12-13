char x1 = 7905;    /* overflow */

short f(short p1, char p2)
{
    int *p;
    struct { int x:1, y:2; } s;
    unsigned long ul;
    char x2[] = { 256, 257 };    /* overflow */
    char *pc;

    s.y = (double)~0UL;                          /* overflow */
    (!~0ul);
    p == ~0ul;
    ul = (double)~0UL + 1.0;                     /* overflow */
    ul = (unsigned long)((double)~0uL + 1.0);

    switch(*p) {
        case ~0Ul:    /* overflow */
            break;
    }

    (pc + 1) + ~0ul;
    -1 | ~0ul;
    -1 % ~0UL;
    1 << ~0ul;
    pc + ~0ul;
    pc - ~0Ul;
    -~0ul;
    ~-1;
    f(~0ul, 256);       /* overflow */

    return ~0uL;    /* overflow */
}
