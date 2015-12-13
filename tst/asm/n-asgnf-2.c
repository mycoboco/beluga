int main(void)
{
    long double ld = 3.14;
    float f = 3.14;
    double d = 3.14;

    f += 1.0;
    d += 1.0;
    ld += 1.0;

    printf("%f, %f, %Lf\n", f, d, ld);
}
