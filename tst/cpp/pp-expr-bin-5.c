#if 0xffffffff + 1
#endif
#if 0x7fffffff + 1          /* overflow */
#endif
#if 0x7ffffffe + 1
#endif
#if -0x7fffffff - 1
#endif
#if -0x7fffffff - 2         /* overflow */
#endif
#if -0x7fffffff -1 -1       /* overflow */
#endif
#if (-0x7fffffff * -1)
#endif
#if (-0x7fffffff-1) * -1    /* overflow */
#endif
#if (-0x7fffffff-1) * 2     /* overflow */
#endif
#if 1 / 0                   /* divide by 0 */
#endif
#if 2 % ZERO                /* divide by 0 */
#endif
#if (-0x7fffffff / -1)
#endif
#if (-0x7fffffff-1) / -1    /* overflow */
#endif
#if -0x7fffffff % -1
#endif
#if (-0x7fffffff-1) % -1    /* overflow */
#endif
#if (-0x7fffffff-1) % -2
#endif
