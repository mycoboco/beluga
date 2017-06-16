/* -Wv */

void f(void)
{
    int a[(0xffffffff00000000)? 0: 1];
}
