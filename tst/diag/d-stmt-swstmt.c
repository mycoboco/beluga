void f1(void)
{
    int x;
    switch(x + 0.1) { case 1: ; }    /* error */
}

void f2(void)
{
    int x;
    switch(x) { ; }    /* error */
}

void f3(void)
{
    char c;
    switch(c) {
        case 1:
            f3();
            break;
        case 2:
            f3();
    }
}
