int main(void)
{
    float f = 3.14;
    double d = 3.14;
    long double ld = 3.14;

    f = -f;
    d = -d;
    ld = -ld;

    printf("%f %f %Lf\n", f, d, ld);
}
