void f1(void)
{
    int i;
    for (i = 0; i < 10; i++)
        f1();
}

void f2(void)
{
    int i;
    for (; i < 10; i++)
        f1();
}

void f3(void)
{
    int i;
    for (;; i++)
        f1();
}


void f4(void)
{
    for (;;)
        f1();
}
