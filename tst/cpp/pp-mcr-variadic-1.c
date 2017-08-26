#define foo1(...) foo #__VA_ARGS__ foo

foo1(arg1,    arg2,
     arg3  )
foo1
(
)

#define fredbar
#define foo2(a, ...) a ## __VA_ARGS__

foo2(fred, bar only)
foo2("foo", "bar" last)
foo2("foo",)
foo2(,)

#define barfred nothing
#define foo3(...) __VA_ARGS__##__VA_ARGS__
foo3(fred, bar)
