
/* from https://stackoverflow.com/questions/5355241/generating-function-declaration-using-a-macro-iteration/5355946#5355946 */

// These need to be updated to handle more than three arguments:
#define PP_HAS_ARGS_IMPL2(_1, _2, _3, N, ...) N
#define PP_HAS_ARGS_SOURCE() MULTI, MULTI, ONE, ERROR

#define PP_HAS_ARGS_IMPL(...) PP_HAS_ARGS_IMPL2(__VA_ARGS__)
#define PP_HAS_ARGS(...)      PP_HAS_ARGS_IMPL(__VA_ARGS__, PP_HAS_ARGS_SOURCE())

#define FOO_ONE(x)     ONE_ARG:    x
#define FOO_MULTI(...) MULTI_ARG:  __VA_ARGS__

#define FOO_DISAMBIGUATE2(has_args, ...) FOO_ ## has_args (__VA_ARGS__)
#define FOO_DISAMBIGUATE(has_args, ...) FOO_DISAMBIGUATE2(has_args, __VA_ARGS__)
#define FOO(...) FOO_DISAMBIGUATE(PP_HAS_ARGS(__VA_ARGS__), __VA_ARGS__)

FOO(1)     // replaced by ONE_ARG:   1
FOO(1, 2)  // replaced by MULTI_ARG: 1, 2
