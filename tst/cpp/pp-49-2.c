#define AND &&

#if 2 || 3 AND 1    /* no paren */
#endif
#if 1 AND 2 || 3    /* no paren */
#endif

#if 2 || (3 AND 1)
#endif
#if (1 AND 2) || 3
#endif

#if (2 || 3) AND 1
#endif
#if 1 AND (2 || 3)
#endif

#if 2 || (3 AND 1) AND 1    /* no paren */
#endif
#if (1 AND 2) || 3 AND 1    /* no paren */
#endif

#if (2 || 3) AND 1 AND 4
#endif
#if 1 AND (2 || 3) AND 4
#endif


#define ANDEXPR 1 AND 2
#define ANDSFX  AND 1
#define ANDPFX  1 AND

#if 2 || ANDEXPR    /* no paren */
#endif
#if ANDEXPR || 3    /* no paren */
#endif

#if 2 || (ANDEXPR)
#endif
#if (ANDEXPR) || 3
#endif

#if (2 || 3) ANDSFX
#endif
#if ANDPFX (2 || 3)
#endif

#if 2 || (ANDEXPR) ANDSFX    /* no paren */
#endif
#if (ANDEXPR) || ANDEXPR    /* no paren */
#endif

#if (2 || 3) ANDSFX ANDSFX
#endif
#if ANDPFX (2 || 3) ANDSFX
#endif


#define OR ||

#if 2 OR 3 AND 1    /* no paren */
#endif
#if 1 AND 2 OR 3    /* no paren */
#endif

#if 2 OR (3 AND 1)
#endif
#if (1 AND 2) OR 3
#endif

#if (2 OR 3) AND 1
#endif
#if 1 AND (2 OR 3)
#endif

#if 2 OR (3 AND 1) AND 1    /* no paren */
#endif
#if (1 AND 2) OR 3 AND 1    /* no paren */
#endif

#if (2 OR 3) AND 1 AND 4
#endif
#if 1 AND (2 OR 3) AND 4
#endif


#define OREXPR 1 OR 2
#define ORSFX  OR 1
#define ORPFX  1 OR

#if ORPFX ANDEXPR    /* no paren */
#endif
#if ANDEXPR ORSFX    /* no paren */
#endif

#if ORPFX (ANDEXPR)
#endif
#if (ANDEXPR) ORSFX
#endif

#if (OREXPR) ANDSFX
#endif
#if ANDPFX (OREXPR)
#endif

#if ORPFX (ANDEXPR) ANDSFX    /* no paren */
#endif
#if (ANDEXPR) ORSFX ANDSFX    /* no paren */
#endif

#if (OREXPR) ANDSFX ANDSFX
#endif
#if ANDPFX (OREXPR) ANDSFX
#endif


#define EXPR ANDPFX OREXPR ANDSFX

#if EXPR    /* no paren x 2 */
#endif
