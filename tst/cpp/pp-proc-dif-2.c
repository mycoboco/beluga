#if 0
#ifndef X
#endif xxx
#endif

#if 0
#ifdef X
#else foo
#endif bar
#endif

#if 0
#if X
#elif
#else foo
#endif bar
#endif

#if 0
#if X
# ifdef x
# else foo
# endif bar
# define f(a) a
f(
#elif
# ifdef x
# else foo
# endif bar
#else foo
#endif bar
#endif
