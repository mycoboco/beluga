void f(float f, double d, long double ld)
{
    printf("%f, %f, %Lf\n", f, d, ld);
}

int main(void)
{
    f(3.14, 3.14, 3.14);
}
