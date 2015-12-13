int i;
struct tag *x;

void f(void) {
    (&i)(*x);
    ("abc")(*x);
    g(*x);
    (&g)(*x);
    (&i, "abc", g)(*x);
    (&i, g, "abc")(*x);
    (&i, g, &g)(*x);
}
