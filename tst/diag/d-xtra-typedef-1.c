typedef int foo;
extern int arr[foo];

struct tag {
    foo:1;
    foo a;
};

void f(foo)
{ }

void g(a)
foo a;
{ a; }

void h(a)
foo:
{
    a;
    foo b;
    a = b;
    foo:
    foo c;
    c;
}

foo:

int i(int a)
{
    switch(a) {
        case 0:
            foo x;
        case 1:
        foo:
            foo y;
        default:
        foo:
            foo z;
    }
    foo:
        foo w;
    return foo;
}
