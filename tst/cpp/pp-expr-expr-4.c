/* -Wv --std=c90 */

#if (1)? 0: (1,2)
#endif

#if (0)? 1: (1,2)    /* warning */
#endif

#if 1 || (1,2)
#endif

#if 0 || (1,2)    /* warning */
#endif

#if 0 && (1,2)
#endif

#if 1 && (1,2)    /* warning */
#endif


#if 0? 1,: 1
#endif

#if 1? 0: 1,
#endif

#if 1? 0: (1,
#endif

#if 1? 0: (1,)
#endif

#if 1 || 1,
#endif

#if 0 && 1,
#endif
