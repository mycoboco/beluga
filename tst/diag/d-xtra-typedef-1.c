typedef int foo;
extern int arr[foo];    /* error */

struct tag {
    foo:1;
    foo a;
};

void f(foo)    /* error */
{ }

void g(a)
foo a;
{ a; }

void h(a)
foo:    /* error */
{
    a;
    foo b;
    a = b;
    foo:      /* error */
    foo c;
    c;
}

foo:    /* error */

int i(int a)
{
    switch(a) {
        case 0:    /* error */
            foo x;
        case 1:
        foo:       /* error */
            foo y;
        default:
        foo:       /* error */
            foo z;
    }
    foo:           /* error */
        foo w;
    return foo;    /* error */
}
