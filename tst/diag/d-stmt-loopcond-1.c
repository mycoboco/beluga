int f(void)
{
    extern int g();

    while (f() || g())
        ;

    do ; while(f() && g());
    for (; f() && g() || f();)
        ;
}
