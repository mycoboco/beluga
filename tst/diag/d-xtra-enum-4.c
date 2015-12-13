typedef enum { A } e1;

int (*gp1)(void) = (void *)(e1)0;
int *gp2 = (e1)0;
void *gp3 = (e1)(1-1);

void f(void)
{
    int (*p1)(void) = (void *)(e1)0;
    int *p2 = (e1)0;

    p1 = (void *)(e1)0;
    p2 = (e1)(1-1);

    p1 = (1)? p1: (void *)(e1)0;
    p1 = (1)? p1: (void *)(e1)1;
}
