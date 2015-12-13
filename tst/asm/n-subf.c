int main(void)
{
    double y;
    float f = 3.14;
    double d = 3.14;
    long double ld = 3.14;

    y = (f - 1.0);
    printf("%f\n", y);
    y = (d - 1.0);
    printf("%f\n", y);
    y = (ld - 1.0);
    printf("%f\n", y);
}
