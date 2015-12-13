void f(void)
{
    int a, b;
    int x, y, z;

    a = a | b;                  /* warning */
    z = (long)x + (long)x;      /* warning */
    z = (short)y + (short)y;    /* warning */
}
