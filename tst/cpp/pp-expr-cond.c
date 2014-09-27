#if FOO? 1 0xffU    /* error, 0 */
line 1
#endif

#if BAR    /* 0 */
line 2
#endif
#if (BAR+1)? 0: (1-1)    /* 0 */
line 3
#endif
line 4
