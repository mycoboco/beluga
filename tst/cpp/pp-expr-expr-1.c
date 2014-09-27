/* -Wv */
#if 3+2,0    /* warning */
omitted
#endif

#if 3+2 	,	       /* warning */
#if (3UL + -2L         /* error */
#endif
#if (3UL + -2L         // error

#endif
#if 3L / 2L)           /* error */
#endif
#if (3L / 2L) + 3 )    /* error */
#endif
#if (FOO)? 1 32L       /* error */
