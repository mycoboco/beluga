#define head foo(
#define foo(x) x
#define xyz(x, y, z) x y z
#define r )

xyz(head, b, r)
xyz(head q, b, r)
xyz(
    head
,
b, r)
xyz(xyz(head, b, r), b, b)
xyz(xyz(head, b, r),
