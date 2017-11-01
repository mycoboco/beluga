#define foo(...) bar(__VA_ARGS__, end)
#define bar(...) __VA_ARGS__

foo()
