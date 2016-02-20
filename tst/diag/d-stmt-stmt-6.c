/* -Wv */

typedef int foo;

void f(int a)
{
    if (a)
        foo i;
    if (a);
    else
}

void g(int a)
{
    while(a)
        foo i;
    for (;a;)
        foo j[] = i;
}

void h(int a)
{
    do foo i; while(a);
    do foo j[] = { i,, } while(a);
    do foo * = { 0, 1, 2 }; while(a);
}

void l(int a)
{
    for (a = 0; a; a++)
}

void m(int a)
{
    switch(a)
        foo i;
    switch(a)
        foo *j = {,};
}

void n(void) {
    switch(a) {
        case 0:
            foo x;
        case 1:
            foo *[] = { x, };
        default:
    }
    lab:
}
