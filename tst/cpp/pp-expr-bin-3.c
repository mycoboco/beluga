#if -1 >> 1           /* warning */
#endif
#if 1 >> -1           /* warning */
#endif
#if 1 >> 31
#endif
#if (1-1) >> 31
#endif
#if 1 >> 32           /* warning */
#endif
#if 1 >> 33           /* warning */
#endif
#if -1 >> 1 == -1     /* warning, 1 */
line1
#endif
#if 1 >> 31U
#endif
#if 1 >> 32U          /* warning */
#endif
#if -1 >> 1U == -1    /* warning, 1 */
line2
#endif
#if -1 << 1           /* warning */
#endif
#if 1 << -100         /* warning */
#endif
#if -1U << 1          /* warning */
#endif
#if 1 << 31           /* warning */
#endif
#if (1-1) << 31
#endif
#if 1 << 32           /* warning */
#endif
#if 2 << 30           /* warning */
#endif
