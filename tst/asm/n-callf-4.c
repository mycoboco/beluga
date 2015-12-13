float x1(int i) { return 3.14; }
double x2(int i) { return 3.14; }
long double x3(int i) { return 3.14; }

int main(void)
{
    x1(0);
    x2(0);
    x3(0);
    printf("%f, %f, %Lf\n", x1(0), x2(0), x3(0));
}
