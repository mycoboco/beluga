void f(void)
{
    void (*pf)(void);

    pf = (1)? pf: 0;
    pf = (1)? pf: (void *)0;
    pf = (1)? pf: (void *)(void *)0;
    pf = (1)? pf: (void *)(int *)0;
    pf = (1)? pf: (void *)pf;
    pf = (1)? pf: ((int *)0 - (int *)0);
    pf = (1)? pf: (void *)((int *)0-(int *)0);

    pf = (1)? pf: (void *)((1)? 0: (int)(3.0-3.0));
    pf = (1)? pf: (void *)((0)? 0: (int)(3.0-3.0));
    pf = (1)? pf: (void *)((0)? 0: (int)3.0-(int)3.0);
}
