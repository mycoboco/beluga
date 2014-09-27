#define xstr(x) str(x)
#define str(x) #x
#define glue(x, y) x##y

xstr
(x)
#line xstr
(x)

xstr(
x)
#line xstr(
x)

xstr(x
)
#line xstr(x
)

xstr(glue
(foo, bar)
)
#line xstr(glue
(foo, bar)
)

xstr(glue(
foo, bar)
)
#line xstr(glue(
foo, bar)
)

xstr(glue(foo
,bar)
)
#line xstr(glue(foo
,bar)
)

xstr(glue(foo,
bar)
)
#line xstr(glue(foo,
bar)
)

xstr(glue(foo, bar
))
#line xstr(glue(foo, bar
))

xstr(glue(foo, bar,
))
#line xstr(glue(foo, bar,
))
