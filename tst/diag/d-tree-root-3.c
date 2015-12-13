extern void v;
extern const void cv;
extern volatile void vv;
void *pv;
const void *pcv;
volatile void *pvv;

extern int g(void);

void f(void)
{
    int x;

    f();
    (void)f();
    (const void)f();

    g();
    (void)g();
    (const void)g();

    f;          /* warning */
    (void)f;

    x, x, x, x, x,, x;
    v, cv, vv, v;
    (v, f());
    (cv, f());
    (pv, pv = 0);          /* warning */
    ((void)pv, pv = 0);

    v;
    (void)v;
    (void)cv;
    (void)vv;

    *pv;
    *pcv;
    (void)*pv;
    (void)*pcv;
    (const void)*pvv;
}
