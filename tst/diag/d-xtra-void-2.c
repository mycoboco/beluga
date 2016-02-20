/* --std=c99 -Wv */

extern void p1;
extern const void cp1;
static void s1;

void f(const void p1, void p2)
{
    int i;
    void *p2;
    const void *cp2;
    void x1;
    const void x2;
    static void s2;

    *p2;
    &*p2;     /* c.v. in C90; okay in C99 */
    *cp2;
    &*cp2;

    &p1;     /* c.v. */
    &cp1;

    p1;     /* undefined */
    cp1;    /* undefined */

    (int)p1;
    (int)cp1;
    (void)p1;     /* undefined */
    (void)cp1;    /* undefined */

    (int)*p2;
    (void)*p2;

    p1 = 0;
    *p2 = 0;
    cp1 = 0;
    *cp2 = 0;
    p1 = p1;
    *p2 = *p2;
    p1 = cp1;
    *p2 = *cp2;
    i = p1;
    i = cp1;

    x1;
    x2;
    p1;
    p2;
    s1;
    s2;
}
