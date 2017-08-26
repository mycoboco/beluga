#define gen foo __VA_ARGS__ bar
#define foo(x) x

gen
foo(__VA_ARGS__)

#define paste(a, b) a##b
#define id(x)  x
#define id2(x) x

paste(__VA_, ARGS__)
id(paste(__VA_, ARGS__))
id2(id(paste(__VA_, ARGS__)))

#define two(x) x x
two(paste(__VA_, ARGS__))
two(__VA_ARGS__)

#define va __VA_ARGS__
paste(va, 2)

#define va1 __VA_
#define va2 ARGS__
#define ext(a, b) paste(a, b)
ext(va1, va2)
