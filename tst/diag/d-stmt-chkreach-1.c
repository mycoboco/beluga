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
            continue;
        }
}

void h(void)
{
    switch(a) {
        break;
        case 0:
            break;
    }
}

void i(void)
{
    int a;
    return;
    a = 0;
    a++;
}
