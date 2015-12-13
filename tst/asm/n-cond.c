int main(void)
{
    int i;

    printf("%d\n", i);
    puts((i > 0)? "i > 0":
         (i == 0)? "i == 0": "i < 0");
}
