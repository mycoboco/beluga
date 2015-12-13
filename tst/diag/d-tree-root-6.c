int f(void)
{
    extern int g();
    int x;

    (f())? g(): g();
    (f())? g(): x;
    (f())? x: g();
    (f())? x: x;                /* warning */

    (f())? (x=x): x;
    (f())? x: (x=x);
    (f())? (x, x): (x, x);      /* warning */
    (f())? (x, x): (x, f());    /* warning */
}
