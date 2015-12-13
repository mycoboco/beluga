int main(void)
{
    float f;
    double d;
    long double ld = 3.141592;

    d = (double)ld + 1.0;
    f = (float)ld + 1.0;

    printf("%f %f\n", d, f);
}
