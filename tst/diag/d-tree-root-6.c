int f(void)
{
    extern int g();
    int x;

    (f())? g(): g();
    (f())? g(): x;
    (f())? x: g();
    (f())? x: x;

    (f())? (x=x): x;
    (f())? x: (x=x);
    (f())? (x, x): (x, x);
    (f())? (x, x): (x, f());
}
