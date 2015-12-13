/* --std=c99 -Wv */

extern void p1;
extern const void cp1;
static void s1;    /* Error */

void f(const void p1, void p2)    /* Error */
{
    int i;
    void *p2;
    const void *cp2;
    void x1;           /* Error */
    const void x2;     /* Error */
    static void s2;    /* Error */

    *p2;
    &*p2;     /* c.v. in C90; okay in C99 */
    *cp2;
    &*cp2;

    &p1;     /* c.v. */
    &cp1;

    p1;     /* undefined */
    cp1;    /* undefined */

    (int)p1;      /* Error */
    (int)cp1;     /* Error */
    (void)p1;     /* undefined */
    (void)cp1;    /* undefined */

    (int)*p2;     /* Error */
    (void)*p2;

    p1 = 0;        /* Error */
    *p2 = 0;       /* Error */
    cp1 = 0;       /* Error */
    *cp2 = 0;      /* Error */
    p1 = p1;       /* Error */
    *p2 = *p2;     /* Error */
    p1 = cp1;      /* Error */
    *p2 = *cp2;    /* Error */
    i = p1;        /* Error */
    i = cp1;       /* Error */

    x1;
    x2;
    p1;
    p2;
    s1;
    s2;
}
