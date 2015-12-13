int main(void)
{
    float f = 3.14;
    double d = 3.14;
    long double ld = 3.14;
    float nf = 0.0 / 0.0;
    double nd = nf;
    long double nl = nd;

    if (f == f) puts("f == f"); else puts("f != f");
    if (nf == nf) puts("problem"); else puts("okay");

    if (d == d) puts("d == d"); else puts("d != d");
    if (nd == nd) puts("problem"); else puts("okay");

    if (ld == ld) puts("ld == ld"); else puts("ld != ld");
    if (nl == nl) puts("problem"); else puts("okay");
}
