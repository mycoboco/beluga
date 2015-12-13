void f(int x)
{
    switch(x) {
        case 0:
            printf("%d\n", 0);
            break;
        case 1:
            printf("%d\n", 1);
            break;
        case 2:
            printf("%d\n", 2);
            break;
        case 3:
            printf("%d\n", 4);
            break;
        case -1:
        case -2:
            return;
    }
}

int main(void)
{
    int i;

    for (i = -2; i < 4; i++)
        f(i);
}
