/* -X */

#define test1(...) foo , ## __VA_ARGS__+bar
#define test2(...) foo,##__VA_ARGS__+bar
#define nothing

test1()
test1(nothing)
test1(foo bar)
test2()
test2(nothing)
test2(foo bar)

#define test3(a, ...) a,  ##   __VA_ARGS__ a

test3(foo)
test3(foo,)
test3(foo, bar)

#define comma
#define test4(a, ...) a comma ## __VA_ARGS__ a

test4(foo)
test4(foo,)
test4(foo,bar)
