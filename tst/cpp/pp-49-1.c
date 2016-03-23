#if 2 || 3 && 1    /* no paren */
#endif
#if 1 && 2 || 3    /* no paren */
#endif

#if 2 || (3 && 1)
#endif
#if (1 && 2) || 3
#endif

#if (2 || 3) && 1
#endif
#if 1 && (2 || 3)
#endif

#if 2 || (3 && 1) && 1    /* no paren */
#endif
#if (1 && 2) || 3 && 1    /* no paren */
#endif

#if (2 || 3) && 1 && 4
#endif
#if 1 && (2 || 3) && 4
#endif


#if 1 + 2 || 2 * 3 && 3 >> 4    /* no paren */
#endif
#if 1 + 2 && 2 * 3 || 3 >> 4    /* no paren */
#endif

#if 2 + 1 || (3 * 4 && 1 >> 1)
#endif
#if (1 + 1 && 2 * 2) || 3 >> 3
#endif

#if (1 + 2 || 3 * 2) && 1 << 1
#endif
#if 1 + 2 && (2 * 2 || 3 << 3)
#endif

#if 2 + 2 || (3 * 3 && 1 << 1) && 1 / 1    /* no paren */
#endif
#if (1 * 1 && 2 * 2) || 3 / 3 && 1 - 1    /* no paren */
#endif

#if (2 + 2 || 3 * 3) && 1 << 1 && !4
#endif
#if ~1 && (2 ^ 3 || 3 & 1) && 4 | 4
#endif
