int main(void)
{
    int i;

    do {
        printf("before: %d\n", i);
        i <<= 2;
        printf("after: %d\n", i);
    } while(i > 0);
}
