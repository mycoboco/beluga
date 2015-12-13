int main(void)
{
    int i = 0;

    label:
    printf("%d\n", i);
    if (++i > 9) goto end;
    goto label;

    end:;
}
