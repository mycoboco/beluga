void f1(void)
{
    double x;

    for (x = 0.0; x > 0; x += 1.0);
    while (x > 0);
    do; while(x > 0);
}

void f2(void)
{
    unsigned long ul;

    for (ul = 0; ul; ul++);
    while (ul > 0);
    do; while(ul > 0);
}

void f3(void)
{
    void *p;

    for (p = 0; p;);
    while (!p);
    do; while(p);
}

void f4(void)
{
    float x;

    for (x = 0; (x = 0.0););
    while (x = 0.0);
    do; while(x = 0);
}
