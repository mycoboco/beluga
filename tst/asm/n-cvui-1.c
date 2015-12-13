int main(void)
{
    unsigned u = -1;

    printf("%u, %u\n", (unsigned)((unsigned char)u+1),
                       (unsigned)((unsigned short)u+1));
}
