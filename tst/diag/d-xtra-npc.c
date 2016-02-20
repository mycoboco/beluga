typedef void (*fp_t)(void);

extern int x;

fp_t gt1 = 0;
fp_t gt2 = (void *)0;
fp_t gt3 = (const void *)0;
fp_t gt4 = (volatile void *)0;
fp_t gt5 = (void * const)0;
fp_t gt6 = (void * volatile)0;
fp_t gt7 = (int *)0;
fp_t gt8 = (1, 0);
fp_t gt9 = (1, (void *)0);
fp_t gt10 = (1)? (void *)0: (void *)1;
fp_t gt11 = (void *)((1)? 0: 1);

fp_t func(fp_t param)
{
    switch(x) {
        case  0: return 0;
        case  1: return (void *)0;
        case  2: return (const void *)0;
        case  3: return (volatile void *)0;
        case  4: return (void * const)0;
        case  5: return (void * volatile)0;
        case  6: return (int *)0;
        case  7: return (1, 0);
        case  8: return (1, (void *)0);
        case  9: return (1)? (void *)0: (void *)1;
        case 10: return (void *)((1)? 0: 1);
    }
}

void f(void)
{
    fp_t t1 = 0;
    fp_t t2 = (void *)0;
    fp_t t3 = (const void *)0;
    fp_t t4 = (volatile void *)0;
    fp_t t5 = (void * const)0;
    fp_t t6 = (void * volatile)0;
    fp_t t7 = (int *)0;
    fp_t t8 = (1, 0);
    fp_t t9 = (1, (void *)0);
    fp_t t10 = (1)? (void *)0: (void *)1;
    fp_t t11 = (void *)((1)? 0: 1);

    func(0);
    func((void *)0);
    func((const void *)0);
    func((volatile void *)0);
    func((void * const)0);
    func((void * volatile)0);
    func((int *)0);
    func((1, 0));
    func((1, (void *)0));
    func((1)? (void *)0: (void *)1);
    func((void *)((1)? 0: 1));
}

void g(void)
{
    int *pi;
    char *pc;
    void *pv;
    void (*pf)(void);

    pc = 1? pi: pv;
    pc = 1? pi: (1, (void *)0);
    pc = 1? pi: (void *)0;

    pf = (void *)0;
    pf = (1, (void *)0);
    pf = pv;

    pi = (1, (void *)0);
    pi = (void *)(1, 0);
}
