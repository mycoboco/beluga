static int **foo;
extern int **foo;

int bar[10];
int bar[];

void f(void)
{
    int **foo;
    int bar;
    {
        extern int **foo;
        extern int bar[100];
    }
}
