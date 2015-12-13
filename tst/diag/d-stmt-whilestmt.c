/* --std=c90 -Wv */
extern int f2(void);

int f1(void)
{
    while(f1()+1)
        f2();
}

int f2(void)
{
    while(1)
        if (f1()) {
            f2();
            break;
            f2();    /* warning */
        }
}
