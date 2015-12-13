extern int f2(void);

int f1(void)
{
    do
        f1();
    while(f2()+1);
}

int f2(void)
{
    do {
        if (f2()+1)
            f1();
        break;
        f2();
    } while(f1());
}
