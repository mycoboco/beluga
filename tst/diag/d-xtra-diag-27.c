char arr[] = { 256 };
int *p;
enum { FOO } *q = p;

void f(void)
{
    enum { FOO } *q = p;

    (void *)0, 0;
    (int *)0, 0;
    (void *)1, 0;
    (void *)q, 0;
}
