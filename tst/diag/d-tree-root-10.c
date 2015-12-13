int f(void)
{
    int g;

    ((void)(g && f()));
    (void)(g && f()), f();
    g=0, (!g || (g = 0)), f();    /* warning */
}
