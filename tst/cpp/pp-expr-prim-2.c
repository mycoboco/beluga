/* -Wv */
#define EXPR 1+BAR
#define ID(x) x
#define EVAL(f) f(BAR)

#if BAR
#endif
#ifdef BAR
#endif
#ifndef BAR
#endif
#if defined BAR
#elif defined(BAR)
#endif
#if 0 / EXPR
#endif
#if EVAL(ID)
#endif

#if 1 || BAR
#endif

#if (unsigned)-1 + sizeof(BAR)
#endif
