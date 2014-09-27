/* -Wv --wchart=long --input-charset=utf8 */

#if L'\t' + -1
4
#endif
#if L'\x10000'
7
#endif
#if L'\xffffffff'
10
#endif
#if L'\x100000000'
13
#endif

#define X(a, b) a ## b
#if X(L, '\xffff')
18
#endif
#if 1+ X(L, '\x100000000')
21
#endif

#if L'\x1234' == 0x1234
25
#endif
