/* -Wv */

void f(void)
{
    int i;
    unsigned u;
    long l;
    unsigned long ul;
    long long ll;
    unsigned long long ull;

    (1)? i + ull: (void *)0;
    (1)? ull + ul: (void *)0;
    (1)? ull + ll: (void *)0;

    (1)? ul + ll: (void *)0;
    (1)? l + ll: (void *)0;
    (1)? u + ll: (void *)0;
}
