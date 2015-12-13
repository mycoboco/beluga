int main(void)
{
    float f = 3.14;
    double d = 3.14;
    long double ld = 3.14;

    printf("%f\n", f / 1.0f);
    printf("%f\n", d / 1.0);
    printf("%Lf\n", ld / 1.0l);
}
