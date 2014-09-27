#if 0xff == 255
2
#endif
#if 0xffff == 65535
5
#endif
#if 0xffffffff == 4294967295
8
#endif
#if 0x100000000
11
#endif
#if 0100 == 100
#elif 0100 == 64
15
#endif
#if 37777777777
18
#endif
#if 40000000000
21
#endif
#if 040000000000
24
#endif
#if 4294967295
27
#endif
#if 4294967296
30
#endif

#define M1(x) 0x100000000
#define M2 040000000000
#define M3(a, b) a ## b
#define M4 M3(429496, 7296)
#if M1(test)
38
#endif
#if M2
41
#endif
#if M4
44
#endif

#if -1 + 0l > 0
48
#else
50
#endif
#if 0L + -1 > 0
53
#else
55
#endif
#if -1+0U > 0
58
#endif
#if 0u+-1 > 0
61
#endif
#if -1+0ul > 0
64
#endif
#if 0UL+-1 > 0
67
#endif
#if 0uL+-1 > 0
70
#endif
#if -1 + 0Ul > 0
73
#endif
#if 3.
#endif
#if 3e-1
#endif
#if .14 + 0l
#endif
#if 3.14.0
#endif
#if 3e
#endif
#if 0xfue-1
#endif
#if 0b111
#endif
#if 0x1ffz
#endif
#if M3(.,14)
#endif
#if M3(314, e)
#endif
#if M3(0x, fue)
#endif
