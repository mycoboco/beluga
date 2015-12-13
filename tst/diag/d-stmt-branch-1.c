void f1(void)
{
    extern int x;
    switch(x) {
        case 1:
            f1();
            return;
            break;
        case 2:
            f1();
            break;
        default:
            { f1(); }
            break;
    }
}

void f2(void)
{
    for (;;) ;    /* warning */

    l1:
    goto l1;    /* warning */

    l2:
    goto l3;
    l3:
    goto l2;    /* warning */
}
