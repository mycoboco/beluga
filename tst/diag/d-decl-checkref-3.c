void f1()
{
    int *p, x1 = 0, x2, x3, x4, x5;    /* p, x1, x2 */
    x2 = 0;
    f1(&x3), x1 = x3;
    f1(x4);
    x5++, p = &x5;
}

void f2()
{
    int a1[10], a2[10] = { 0 }, a3[10] = { 0 };    /* a1, a3 */
    a1[0] = 0;
    f2(&a2);
}

void f3(int p1, int p2, int p3)    /* p1, p2 */
{
    p2 = 0;
    f2(&p3);
}

void f4()
{
    static int x1, x2 = 0, x3;    /* x1, x2, x3 */
    x1 = 0;
}

static int s1, s2 = 0;    /* s1, s2 */
void f5() { s1 = 0; }

int g1, g2 = 0;
void f6() { g1 = 0; }

int f7(void) { return x; }
void f8(void) { x = 0; }

static int g = t;    /* g */

void f9(void) { struct { int x; } x = 0; }    /* x */
