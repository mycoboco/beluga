/* -Wv */

extern int a;

void f(void)
{
    switch(a) {
        case 0:
            break;
        case 1:
            break;
        case 2:
            break;
        case 3:
            break;
    }
    f();
}

void g(void)
{
    while(1)
        if (a)
            continue;
        else {
            break;
            continue;    /* warning */
        }
}

void h(void)
{
    switch(a) {
        break;    /* warning */
        case 0:
            break;
    }
}

void i(void)
{
    int a;
    return;
    a = 0;    /* warning */
    a++;
}
