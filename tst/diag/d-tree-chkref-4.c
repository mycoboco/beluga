struct t { int l; int m: 2; };
struct s { struct t m; };

int g();

int f(int x1, int x2, int x3,
      struct t x4, struct t x5, struct t x6, struct t x7,
      struct s x8, struct s x9, struct s x10, struct s x11, struct s x12,
      unsigned char x13)
{
    int y;
    struct s f1();
    unsigned char *p;

    y = 1 + x1;
    y = *(&x2 + x1);
    y = (&x3 + (&x2 - &x1))[x1];
    x4 = x4;
    y = x5.l;
    y = x6.m;
    x7.m = y;
    x7 = x8.m;
    y = x9.m.m;
    x10.m.m = y;
    y = (&x11 + x1 + g())[1].m.m;
    y = (&x10 - &x9 + &x12)[g()].m.m;
    y = f1().m.m;
    x1 = *(g(), p = &x13);
}

void g(void)
{
    int y;
    struct s f1();

    f1().m.m = y;
}
