struct t { int p; int t: 2; };
struct s { struct t s; };
struct u { int p; struct t u; };
struct v { int v:2; };

int f(struct t x1,
      struct v x2,
      struct t x3,
      struct u x4,
      struct s x5,
      struct t x6,
      struct v x7,
      struct t x8,
      struct u x9,
      struct s x10,
      struct t x11)
{
    int y;
    struct u xy;

    x1.t = y;
    x2.v = y;
    x3 = xy.u;
    x4.u.t = y;
    x5.s.t = y;

    y = x6.t;
    y = x7.v;
    xy.u = x8;
    y = x9.u.t;
    y = x10.s.t;
}
