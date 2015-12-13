int main(void)
{
    long double ld = 0.1l;
    long double ld2;

    ld2 = ld;
    if (ld == ld2)
        puts("okay");

    ld = (float)ld;
    if (ld != ld2)
        puts("okay");
}
