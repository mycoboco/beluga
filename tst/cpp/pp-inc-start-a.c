/* -Wv -I. */

#define gt >
#define cmt /* ... */
#define nothing(a, b, c)
#define string "pp-inc-start-c.c"

#include <pp-inc-start-b.c>nothing
#include <pp-inc-start-c.c> nothing

#include nothing(1, 2, 3) <pp-inc-start-b.c>nothing





#include nothing(1, 2, 3) < pp-inc-start-c.c gt nothing

#include nothing
#include cmt string nothing
