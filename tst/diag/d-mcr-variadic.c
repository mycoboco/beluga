/* -Wv -X */

#define foo1(x, ...) x ## , ## __VA_ARGS__ end
#define foo2(...)    , ## __VA_ARGS__ end
#define foo3(...)    , ## __VA_ARGS__ ## end

foo1(foo)
foo1(foo,)
foo1(foo, bar)
foo1(foo, bar, fred)

foo2()
foo2(foo)
foo2(foo, bar)

foo3()
foo3(foo)
foo3(foo, bar)
