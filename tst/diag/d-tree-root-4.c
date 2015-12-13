extern int g(void);
extern int *h(void);
extern double d(void);
extern unsigned u(void);

void f(void)
{
    const unsigned char uc;

    uc;                         /* warning */
    (int)uc;                    /* warning */
    (unsigned)uc;               /* warning */
    (unsigned char)uc;          /* warning */
    (const int)uc;              /* warning */
    (const unsigned char)uc;    /* warning */

    g();
    (int)g();              /* warning */
    (const int)g();        /* warning */
    (double)g();           /* warning */
    (short)g();            /* warning */
    (unsigned char)g();    /* warning */
    (void)g();
    (const void)g();

    h();
    (void)h();
    (int *)h();       /* warning */
    (int)h();         /* warning */
    (void *)h();      /* warning */
    (double *)h();    /* warning */

    d();
    (void)d();
    (double)d();      /* warning */
    (unsigned)d();    /* warning */

    u();
    (void)u();
    (unsigned)u();    /* warning */
    (float)u();       /* warning */
}
