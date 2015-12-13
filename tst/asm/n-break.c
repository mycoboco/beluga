int main(void)
{
    int i, j;

    for (i = 0; i < 10; i++) {
        for (j = 0; j < 10; j++) {
            if (i == j) {
                printf("%d, %d\n", i, j);
                break;
            } else
                continue;
        }
        if (j == 8)
            break;
    }
}
