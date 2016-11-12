/* -Wv --std=c90 */

#define fred bar(a,)
#define bar(x)
#define foo(x) x

foo(fred)

#undef fred
#define fred bar(a, b)

foo(fred)
