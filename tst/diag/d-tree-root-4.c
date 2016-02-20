extern int g(void);
extern int *h(void);
extern double d(void);
extern unsigned u(void);

void f(void)
{
    const unsigned char uc;

    uc;
    (int)uc;
    (unsigned)uc;
    (unsigned char)uc;
    (const int)uc;
    (const unsigned char)uc;

    g();
    (int)g();
    (const int)g();
    (double)g();
    (short)g();
    (unsigned char)g();
    (void)g();
    (const void)g();

    h();
    (void)h();
    (int *)h();
    (int)h();
    (void *)h();
    (double *)h();

    d();
    (void)d();
    (double)d();
    (unsigned)d();

    u();
    (void)u();
    (unsigned)u();
    (float)u();
}
