int main(void)
{
    int i = 0x12345678;
    unsigned u = 0x12345678;

    printf("%x\n", i ^ (int)u);
    printf("%x\n", i ^ u);
}
