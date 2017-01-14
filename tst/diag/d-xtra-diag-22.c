typedef int x;
int *x;

int foo[10];
int foo[];

void g(void)
{
    extern int *bar;
    int **fred(void);
}

void func(void)
{
    int *poo;
    int poo[];
    extern int *foo;
    int *bar;
    {
        extern int **bar;
    }
}

void h(void)
{
    fred(1, 2, 3);
}
