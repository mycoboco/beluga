/* -Wv --std=c90 */

#define fred bar()
#define bar(x)
#define foo(x) x

foo(fred)
