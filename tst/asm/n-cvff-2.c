int main(void)
{
    float f = 3.141592;
    double d = 3.141592;
    long double ld = 3.141592;

    ld = (long double)f + 1.0;
    ld = (long double)d + 1.0;

    printf("%Lf\n", ld);
}
