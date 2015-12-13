int main(void)
{
    unsigned u = 0xfffffffe;
    int i;

    i = (unsigned char)(u + 1);
    printf("%x\n", (unsigned)i);
    i = (char)(u + 1);
    printf("%d\n", i);
}
