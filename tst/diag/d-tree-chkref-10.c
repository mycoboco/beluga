void f(void)
{
    int *g();
    int x, y, z, w;

    y = *(&x + (&z - &w));
    y = z;
}
