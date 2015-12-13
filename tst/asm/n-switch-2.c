int doprime(int x)
{
    int i;

    if (x <= 10)
        puts("something goes wrong");

    for (i = 2; i < x; i++)
        if (x % i == 0)
            return 0;

    return 1;
}

int prime(int x)
{
    switch(x) {
        default:
            if (doprime(x))
        case 1:
        case 2:
        case 3:
        case 5:
        case 7:
                return 1;
            else
        case 4:
        case 6:
        case 8:
        case 9:
        case 10:
                return 0;
    }
}

int main(void)
{
    int i;

    for (i = 1; i < 20; i++)
        if (prime(i))
            printf("%d\n", i);
}
