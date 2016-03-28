#if 0 | 0
#endif
#if 1 & 1
#endif
#if 1 ^ 0
#endif

#if 0 | 0 == 0    /* no paren */
#endif
#if 1 & 1 != 1    /* no paren */
#endif
#if 1 ^ 0 < 0     /* no paren */
#endif
#if 0 | 0 > 0     /* no paren */
#endif
#if 1 & 1 <= 1    /* no paren */
#endif
#if 1 ^ 0 >= 0    /* no paren */
#endif

#if 0 < 0 | 0     /* no paren */
#endif
#if 1 > 1 & 1     /* no paren */
#endif
#if 0 >= 1 ^ 0    /* no paren */
#endif
#if 0 != 0 | 0    /* no paren */
#endif
#if 1 == 1 & 1    /* no paren */
#endif
#if 0 <= 1 ^ 0    /* no paren */
#endif

#if 0 | 0 == 1 & 1     /* no paren */
#endif
#if 1 ^ 0 == 1 | 1     /* no paren */
#endif
#if 0 == 0 & 1 == 1    /* no paren x 2 */
#endif
#if 1 > 1 ^ 0 < 0      /* no paren x 2 */
#endif

#if 0 | (0 == 0)
#endif
#if (1 & 1) != 1
#endif
#if (0 | 0) == 1 & 1    /* no paren */
#endif
#if 0 | 0 == (1 & 1)    /* no paren */
#endif
#if 0 | (0 == 1) & 1
#endif
#if (0 | 0) == (1 & 1)
#endif
#if (0 == 0) & (1 == 1)
#endif
#if 0 == (0 & 1) == 1
#endif

#if 1 & 1 == 0 && 0 | 0 == 1 || 0 == 1 ^ 1    /* no paren x 4 */
#endif

#define BIT 1 & 1
#define CMP 0 < 1
#define EXPR 1 & 1 < 0

#if BIT < 0     /* no paren */
#endif
#if 0 == BIT    /* no paren */
#endif
#if 1 & CMP     /* no paren */
#endif
#if CMP ^ 1     /* no paren */
#endif
#if EXPR        /* no paren */
#endif
#if (BIT) < 0
#endif
#if 0 == (BIT)
#endif
#if 1 & (CMP)
#endif
#if (CMP) ^ 1
#endif
#if (EXPR)    /* no paren */
#endif
