/* -Wv --std=c90 --target-endian=little --input-charset=utf8 --wide-exec-charset=UCS-2LE */

#if '' + 1
4
#endif
#if 1 + '
7
#endif
#if 'a' == 97
10
#endif

#if '가'
14
#endif
#if L'가' == 0xAC00
17
#endif
#if L'가나'
20
#endif
#if L'a가'
23
#endif

#if '\xff' + 1
27
#endif
