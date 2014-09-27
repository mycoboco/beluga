#if 1 || 0    /* 1 */
line1
#endif
#if 1 && 0    /* 0 */
line2
#endif
#if -0x7fffffff - (-1U == ~0)    /* warning */
#endif
#if (-0x7fffffff-1) - (-1U == ~0)    /* overflow */
#endif
#if (-0x7fffffff-1) - (-1U != ~0)    /* warning */
#endif
#if ~0U > 0xfffffffe     /* 1 */
line1
#endif
#if -1U >= 0xffffffff    /* 1 */
line2
#endif
#if ~0U < 0xffffffff    /* 0 */
line3
#endif
#if -1U <= 0xffffffff    /* 1 */
line4
#endif
#if 1U >> -1    /* warning */
#endif
#if 1U >> 31
#endif
#if 1U >> 32    /* warning */
#endif
#if 1U >> -1U    /* warning */
#endif
#if 1U >> 31U
#endif
#if 1U >> 32U    /* warning */
#endif
#if 1U << -1    /* warning */
#endif
#if 1U << 31
#endif
#if 1U << 32    /* warning */
#endif
#if 1U << -1U    /* warning */
#endif
#if 1U << 31U
#endif
#if 1U << 32U    /* warning */
#endif
#if 1U & 0    /* 0 */
line5
#endif
#if 1U | 0    /* 1 */
line6
#endif
#if 1U ^ 1U    /* 0 */
line7
#endif
#if 0x7fffffffU + 2
#endif
#if 0xffffffff + 1
#endif
#if 0 - 1
#endif
#if (-0x7fffffff-1) - 1U    /* warning */
#endif
#if -0x7fffffff - 2U    /* warning */
#endif
#if (0x3fffffff+1U) * 2    /* overflow */
#endif
#if 0x7fffffffU * -1    /* warning */
#endif
#if 1U / 0    /* divide by 0 */
#endif
#if 1 / 0U    /* divide by 0 */
#endif
#if 1U % 0    /* divide by 0 */
#endif
#if (-0x7fffffff-1) / -1U    /* warning */
#endif
#if (-0x7fffffff-1) % -1U    /* warning */
#endif
