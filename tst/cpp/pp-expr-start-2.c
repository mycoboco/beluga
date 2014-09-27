#if a + b / 12 )                   /* error */
#endif
#if defined(x) / 1 + '\0' "abc"    /* error */
#endif

#define STR L"str"
#if defined(x) / 1 + '\0' STR      /* error */
#endif
#if a + b +                        /* error */
#endif
#if a + b C                        /* error */
#endif

#define X
#if a + b X
#endif

#if a + b 0b11                     /* error */
#endif
#if a + b 0                        /* error */
#endif
#if a + b #                        /* error */
#endif
#if a + b ##                       /* error */
#endif
