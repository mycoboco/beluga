/* -Wv --std=c90 --plain-char=unsigned */

#if '\a' == 7
4
#endif
#if '\b' == 8
7
#endif
#if '\f' == 12
10
#endif
#if '\n' == 10
13
#endif
#if '\r' == 13
16
#endif
#if '\t' == 9
19
#endif
#if '\v' == 11
22
#endif
#if '\'' == 39
25
#endif
#if '\"' == '"'
28
#endif
#if '\\' == 92
31
#endif
#if '\?' == '?'
34
#endif

#if '\xff' == 255
38
#endif
#if '\x100'
41
#endif
#if '\x0' + -1 > 0
44
#else
46
#endif
#if '\xz'
49
#endif
#if '\x0z'
52
#endif

#if '\0' == 0
56
#endif
#if '\7' == 7
59
#endif
#if '\8'
62
#endif
#if '\0400'
65
#endif
#if '\0128'
68
#endif
#if '\012x'
71
#endif
#if '\377'
74
#endif
#if '\400'
77
#endif
#if '\!'
80
#endif

#define A '\x100'
#define B A

#if B
87
#endif
