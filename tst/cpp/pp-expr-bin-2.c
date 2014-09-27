#if (0 - 1) + (0xFFU + -1)    /* warning */
#endif
#if 0xffU + -1                /* warning */
#endif
#if (0xFF << 1U) + -1
#endif
#if (0xFFU << 1) + -1         /* warning */
#endif

#define foo(x) (0xFF ## x << 1U) + -1
#if nodef + foo(U)    /* warning */
#endif

#if -1 || 0
#endif
#if -1 || 0U
#endif
#if -1 ^ -1    /* 0 */
line1
#endif

#if 0 | 1    /* 1 */
line2
#endif

#if 1 & 0    /* 0 */
line3
#endif

#if (0-1) == -1    /* 1 */
line4
#endif

#if (0-1) != -1    /* 0 */
line5
#endif
