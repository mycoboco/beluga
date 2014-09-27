#if "string"
#endif
#if L"string"
#endif
#define STR(x) #x
#if STR(wow !!!)
#endif
#if 'A'
9
#endif
#if L'A'
12
#endif

#if 1 +
#endif
#define ONEPLUS 1 +
#if ONEPLUS
#endif
#if ONEPLUS /* test */
#endif
#if ONEPLUS [
#endif
#if 1 + @
#endif
#if 1 + \ /* ... */
#endif

#if UNDEF
30
#else
32
#endif

#if defined /* ... */ (UNDEF)
36
#else
38
#endif
#if defined(ONEPLUS)
41
#endif
#if defined ONEPLUS
44
#endif
#if defined !
#endif
#if defined (!
#endif
#if defined UNDEF !
#endif
#if defined (UNDEF!)
#endif
#if defined ( /* ... */ ONEPLUS
#endif

#define LPAREN (
#define DEF defined LPAREN
#define RPAREN )
#if DEF x
#endif
#if DEF x RPAREN
#endif
#if defined LPAREN x )
#endif
