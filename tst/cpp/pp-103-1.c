/* -X */

#define has_comma(...) helper(__VA_ARGS__, 1)
#define helper(a, b, ...) 0 ## b

#define nonstd(...) has_comma(dummy, ## __VA_ARGS__)

#if nonstd()
non-conforming
#else
conforming
#endif
