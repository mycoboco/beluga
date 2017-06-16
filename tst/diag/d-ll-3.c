/* -Wv */

void f(void)
{
    long l;
    unsigned long ul;
    long long ll;
    unsigned long long ull;

    long unsigned long ull2;
    long signed long ll2;
    long long unsigned ull3;
    long long signed ll3;

    (1)? l: (void *)0;
    (1)? ul: (void *)0;
    (1)? ll: (void *)0;
    (1)? ull: (void *)0;
    (1)? ull2: (void *)0;
    (1)? ll2: (void *)0;
    (1)? ull3: (void *)0;
    (1)? ll3: (void *)0;
}

void g(void)
{
    unsigned long long long ulll1;
    long long unsigned long ulll2;
    long long char llc;
    long char long lcl;
    char long long cll;
    long double long ldl;
    double long long dll;
    char double long long cdll;
    long long double char lldc;
    unsigned long long signed ulls;
}
