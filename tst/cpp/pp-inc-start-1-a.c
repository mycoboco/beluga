/* -Wv -I. */

#define gt >
#define cmt /* ... */
#define nothing(a, b, c)
#define string "pp-inc-start-1-c.c"

#include <pp-inc-start-1-b.c>nothing
#include <pp-inc-start-1-c.c> nothing

#include nothing(1, 2, 3) <pp-inc-start-1-b.c>nothing





#include nothing(1, 2, 3) < pp-inc-start-1-c.c gt nothing

#include nothing
#include cmt string nothing
