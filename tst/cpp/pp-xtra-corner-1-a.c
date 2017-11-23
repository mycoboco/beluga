
/* from WG21's N3882 */

#define cheeseburger

#define can_haz( x, y ) defined x ## y
#if can_haz( cheese, burger )
#error generated defined works with external suppression
#endif

#define d( x ) defined x
#if d( cheeseburger )
#error "define x" suppresses argument expansion
#endif


#define nullary()
#define assert_empty(x) nullary(x)

assert_empty(
#define x // UB: directive inside argument list
x // OK if directive was executed before replacement finishes
)


#define a(x) x

a(
#include "pp-xtra-corner-1-b.c"
)
