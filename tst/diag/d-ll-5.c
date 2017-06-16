/* -Wv */

void f(void)
{
    int *pi;

    pi = (1)? pi: 0LL;
    pi = (1)? pi: 0ULL;
    pi = (1)? pi: 0xffffffff00000000;
    pi = (1)? pi: 0x1000000000000000;
}
