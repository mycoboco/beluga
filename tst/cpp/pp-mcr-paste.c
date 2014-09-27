#define x(xy, yx, f, g) xy ## yx ## f ## g
x(x, y, foo xy, )

#define z(a, b) a ## b
#define foo(x) x
#define bar foobar

foo(z(b, ar))
