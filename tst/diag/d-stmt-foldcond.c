void f1(void)
{
    int i;
    for (i = 0; i < 10; i++);     /* folded */
}

void f2(void)
{
    int i;
    for (i = 0; i < i; i++);      /* not folded */
}

void f3(void)
{
    int i;
    for (i = 0; i < 10U; i++);    /* not folded */
}

void f4(void)
{
    int i;
    for (i = 0; 10 > i; i++);     /* not folded */
}
