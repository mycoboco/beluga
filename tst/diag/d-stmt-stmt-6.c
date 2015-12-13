/* -Wv */

typedef int foo;

void f(int a)
{
    if (a)
        foo i;    /* error */
    if (a);
    else
}    /* error */

void g(int a)
{
    while(a)
        foo i;    /* error */
    for (;a;)
        foo j[] = i;    /* error */
}

void h(int a)
{
    do foo i; while(a);    /* error */
    do foo j[] = { i,, } while(a);    /* error */
    do foo * = { 0, 1, 2 }; while(a);    /* error */
}

void l(int a)
{
    for (a = 0; a; a++)
}    /* error */

void m(int a)
{
    switch(a)
        foo i;    /* error */
    switch(a)
        foo *j = {,};
}

void n(void) {
    switch(a) {
        case 0:    /* error */
            foo x;
        case 1:
            foo *[] = { x, };
        default:    /* error */
    }
    lab:    /* error */
}
